#include "tippingworker.h"

#include <QtGlobal>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QVariant>
#include <QJsonArray>
#include <QStringList>
#include <QList>
#include <cmath>
#include "txlogentry.h"

namespace {
QString requiredBotMention()
{
    return qEnvironmentVariable("GRIN_CHAIN_TYPE") == "testnet" ? "@grin_mw_test_bot" : "@grin_mw_bot";
}

QString normalizeCommandText(const QString &text)
{
    QString normalized = text.trimmed();
    if (!normalized.startsWith('/')) {
        return normalized;
    }

    int firstSpace = normalized.indexOf(' ');
    QString firstToken = (firstSpace == -1) ? normalized : normalized.left(firstSpace);
    int atIndex = firstToken.indexOf('@');
    if (atIndex == -1) {
        return normalized;
    }

    QString mention = firstToken.mid(atIndex);
    if (mention.compare(requiredBotMention(), Qt::CaseInsensitive) != 0) {
        return QString();
    }

    QString cleaned = firstToken.left(atIndex);
    if (cleaned.isEmpty()) {
        return normalized;
    }

    QString remainder = (firstSpace == -1) ? QString() : normalized.mid(firstSpace);
    return cleaned + remainder;
}

constexpr qlonglong nanogrinPerGrin = 1000000000LL;

QString formatGrin(qlonglong nanogrin)
{
    bool negative = nanogrin < 0;
    qlonglong absolute = negative ? -nanogrin : nanogrin;
    qlonglong whole = absolute / nanogrinPerGrin;
    qlonglong fraction = absolute % nanogrinPerGrin;

    QString result = QString::number(whole);
    if (fraction != 0) {
        QString fractional = QString::number(fraction).rightJustified(9, '0');
        while (fractional.endsWith('0')) {
            fractional.chop(1);
        }
        result += QString(".%1").arg(fractional);
    }

    if (negative) {
        result.prepend("-");
    }

    return result;
}

bool parseTipAmount(const QString &input, qlonglong &nanogrin, QString &errorMessage)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        errorMessage = QString("Please provide a valid tip amount. You entered: %1").arg(input);
        return false;
    }

    if (trimmed.contains(',')) {
        errorMessage = QString("Please use a dot as decimal separator. You entered: %1").arg(input);
        return false;
    }

    bool ok = false;
    double value = trimmed.toDouble(&ok);
    if (!ok || !std::isfinite(value)) {
        errorMessage = QString("Please enter a numeric tip amount. You entered: %1").arg(input);
        return false;
    }

    if (value < 0.1) {
        errorMessage = QString("Minimum tip is 0.1 GRIN. You entered: %1").arg(input);
        return false;
    }

    nanogrin = static_cast<qlonglong>(value * nanogrinPerGrin + 0.5);
    return true;
}
}

TippingWorker::TippingWorker(TelegramBot *bot, QSettings *settings, WalletOwnerApi *walletOwnerApi) :
    m_bot(bot),
    m_settings(settings),
    m_db(nullptr),
    m_walletOwnerApi(walletOwnerApi),
    tippingAccountLabel("tipping"),
    walletPassword("test"),
    m_pendingDepositTimer(nullptr)
{
}

bool TippingWorker::init()
{
    QString dbPath;
    QString dataDir = qEnvironmentVariable("DATA_DIR");

    if (dataDir.isEmpty()) {
        dbPath = QCoreApplication::applicationDirPath() + "/etc/database/tipping.db";
    } else {
        dbPath = QDir(dataDir).filePath("etc/database/tipping.db");
    }

    qDebug() << "DB Pfad Tipping:" << dbPath;

    m_db = new TippingDatabase(dbPath, this);
    m_db->initialize();

    walletPassword = m_settings->value("wallet/password").toString();
    tippingAccountLabel = "tipping";

    if (!m_walletOwnerApi) {
        qWarning() << "Wallet owner API is not initialized";
        return false;
    }

    if (!ensureTippingAccount()) {
        qWarning() << "Tipping account preparation failed";
        return false;
    }

    m_pendingDepositTimer = new QTimer(this);
    connect(m_pendingDepositTimer, &QTimer::timeout, this, &TippingWorker::checkPendingDeposits);
    connect(m_pendingDepositTimer, &QTimer::timeout, this, &TippingWorker::checkPendingWithdrawConfirmations);
    m_pendingDepositTimer->start(30 * 1000);

    QList<PendingWithdrawRecord> storedWithdraws = m_db->pendingWithdrawals();
    for (const PendingWithdrawRecord &withdraw : storedWithdraws) {
        if (withdraw.slateId.isEmpty()) {
            continue;
        }
        m_pendingWithdraws.insert(withdraw.slateId, withdraw);
        qDebug() << "init: restored pending withdraw" << withdraw.slateId << "amount" << withdraw.amount;
    }

    return true;
}

bool TippingWorker::handleUpdate(TelegramBotUpdate update)
{
    if (!update || update.isNull() || update->type != TelegramBotMessageType::Message) {
        return false;
    }

    TelegramBotMessage &message = *update->message;
    QString text = normalizeCommandText(message.text);

    if (!message.document.fileId.isEmpty() && !message.document.fileName.isEmpty()) {
        return handleSlatepackDocument(message);
    }

    if (text.isEmpty()) {
        return false;
    }

    if (text.contains("BEGINSLATEPACK") && text.contains("ENDSLATEPACK")) {
        return handleSlatepackText(message, text);
    }

    const QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return false;

    const QString senderId = QString::number(message.from.id);
    if (!senderId.isEmpty() && !message.from.username.isEmpty()) {
        m_db->ensureUserRecord(senderId, message.from.username);
    }

    const QString cmd = parts[0];
    const QString sender = userLabel(message);

    if (cmd == "/tipping") {
        QString path;
        QString dataDir = qEnvironmentVariable("DATA_DIR");

        if (dataDir.isEmpty()) {
            path = QCoreApplication::applicationDirPath() + "/etc/messages/tipping.txt";
        } else {
            path = QDir(dataDir).filePath("etc/messages/tipping.txt");
        }

        qDebug()<<path;

        QString content = readFileToString(path);
        if (content.isEmpty()) {
            content = "The tipping guide is currently unavailable. Please try again later.";
        }

        sendUserMarkdownMessage(message, content, false);
        return true;
    }

    if (cmd == "/adminamounts") {
        if (!isAdmin(message.from.id)) {
            sendUserDirectMessage(senderId, "You are not authorized to run this command.", false);
            return true;
        }
        sendUserDirectMessage(senderId, handleAdminAmountsCommand(), false);
        return true;
    }

    if (cmd == "/adminbalance") {
        if (!isAdmin(message.from.id)) {
            sendUserDirectMessage(senderId, "You are not authorized to run this command.", false);
            return true;
        }
        QList<BalanceRecord> balances = m_db->listBalances();
        if (balances.isEmpty()) {
            sendUserDirectMessage(senderId, "No balances available yet.", false);
            return true;
        }
        QStringList lines;
        lines << QString("Balances (%1 entries):").arg(balances.size());
        for (const BalanceRecord &entry : balances) {
            QString label = m_db->usernameByUserId(entry.userId);
            if (label.isEmpty()) {
                label = entry.userId;
            } else if (!label.startsWith("@")) {
                label = QString("@%1").arg(label);
            }
            lines << QString("%1: %2 GRIN").arg(label).arg(formatGrin(entry.balance));
        }
        sendUserDirectMessage(senderId, lines.join("\n"), false);
        return true;
    }

    if (cmd == "/deposit") {
        if (parts.size() != 2) {
            sendUserDirectMessage(senderId, "Usage: /deposit <amount in GRIN>", false);
            return true;
        }
        bool ok;
        int amount = parts[1].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserDirectMessage(senderId, "Please provide a positive amount.", false);
            return true;
        }
        QString depositResponse = handleDepositCommand(senderId, amount, message);
        if (!depositResponse.isEmpty()) {
            sendUserDirectMessage(senderId, depositResponse, false);
        }
        return true;
    }

    if (cmd == "/withdraw") {
        if (parts.size() != 2) {
            sendUserDirectMessage(senderId, "Usage: /withdraw <amount in GRIN>", false);
            return true;
        }
        bool ok;
        int amount = parts[1].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserDirectMessage(senderId, "Please provide a positive amount.", false);
            return true;
        }
        QString withdrawResponse = handleWithdrawCommand(senderId, amount, message);
        if (!withdrawResponse.isEmpty()) {
            sendUserDirectMessage(senderId, withdrawResponse, false);
        }
        return true;
    }

    if (cmd == "/tip") {
        if (parts.size() != 3) {
            sendUserMessage(message, "Usage: /tip @user <amount>", false);
            return true;
        }
        QString toUser = parts[1];
        if (toUser.startsWith("@")) {
            toUser = toUser.mid(1);
        }

        qlonglong tipNanogrin = 0;
        QString tipError;
        if (!parseTipAmount(parts[2], tipNanogrin, tipError)) {
            sendUserMessage(message, tipError, false);
            return true;
        }

        QString recipientId = resolveRecipientId(toUser, message);
        if (recipientId.isEmpty()) {
            sendUserMessage(message,
                            "I could not resolve that recipient. Ask them to send /tipping so I can learn their ID, then try again.",
                            false);
            return true;
        }

        if (recipientId == senderId) {
            sendUserMessage(message, "You cannot tip yourself.", false);
            return true;
        }

        QString recipientLabel = m_db->usernameByUserId(recipientId);
        if (recipientLabel.isEmpty()) {
            recipientLabel = toUser;
        }
        QString displayRecipient;
        if (!recipientLabel.isEmpty()) {
            displayRecipient = recipientLabel.startsWith("@") ? recipientLabel : QString("@%1").arg(recipientLabel);
        } else {
            displayRecipient = recipientId;
        }

        if (m_db->getBalance(senderId) < tipNanogrin) {
            sendUserMessage(message, "Insufficient balance.", false);
            return true;
        }

        if (!m_db->updateBalance(senderId, -tipNanogrin)) {
            sendUserMessage(message, "Insufficient balance.", false);
            return true;
        }

        if (!m_db->updateBalance(recipientId, tipNanogrin)) {
            qWarning() << "Failed to credit tip recipient" << recipientId;
            // rollback sender balance
            m_db->updateBalance(senderId, tipNanogrin);
            sendUserMessage(message, "Unable to credit recipient; tip cancelled.", false);
            return true;
        }
        m_db->recordTransaction(senderId, recipientId, tipNanogrin, "tip", displayRecipient);
        sendUserMessage(message, QString("%1 GRIN sent to %2.").arg(formatGrin(tipNanogrin)).arg(displayRecipient), false, false);
        return true;
    }

    if (cmd == "/balance") {
        qlonglong bal = m_db->getBalance(senderId);
        sendUserDirectMessage(senderId, QString("Your current balance: %1 GRIN").arg(formatGrin(bal)), false);
        return true;
    }

    if (cmd == "/ledger") {
        sendUserDirectMessage(senderId, handleLedgerCommand(senderId, sender), false);
        return true;
    }

    if (cmd == "/opentxs") {
        sendUserDirectMessage(senderId, handleOpenTransactionsCommand(sender), false);
        return true;
    }

    return false;
}

bool TippingWorker::handleSlatepackDocument(TelegramBotMessage &message)
{
    if (!activateTippingWalletAccount()) {
        sendUserMessage(message, "Wallet account could not be activated.", false);
        return true;
    }

    TelegramBotFile file = m_bot->getFile(message.document.fileId);
    QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
    QString slatepack = downloadFileToQString(QUrl(link));

    if (slatepack.isEmpty()) {
        sendUserMessage(message, "Slatepack could not be downloaded.", false);
        return true;
    }

    Slate slate;
    Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
    if (!res.unwrapOrLog(slate)) {
        sendUserMessage(message, QString("Error: %1").arg(res.errorMessage()), false);
        return true;
    }

    SlateState state = Slate::slateStateFromString(slate.sta());
    QString slateId = slate.id();
    if (state == SlateState::I2) {
        PendingDepositRecord pending;
        if (slateId.isEmpty() || !m_db->pendingDeposit(slateId, pending)) {
            qDebug() << "handleSlatepackDocument: ignoring I2 slate" << slateId << "not pending";
            return false;
        }
        Result<QString> result = handleSlateI2State(slate, message);
        QString info;
        if (!result.unwrapOrLog(info)) {
            sendUserMessage(message, QString("Error: %1").arg(result.errorMessage()), false);
            return true;
        }
        sendUserMessage(message, info, false);
        return true;
    }

    if (state == SlateState::S2) {
        if (slateId.isEmpty() || !m_pendingWithdraws.contains(slateId)) {
            qDebug() << "handleSlatepackDocument: ignoring S2 slate" << slateId << "not pending withdraw";
            return false;
        }
        PendingWithdrawRecord pending = m_pendingWithdraws.value(slateId);
        Result<QString> result = handleSlateS2State(slate, message, pending);
        QString info;
        if (!result.unwrapOrLog(info)) {
            sendUserMessage(message, QString("Error: %1").arg(result.errorMessage()), false);
            return true;
        }
        sendUserMessage(message, QString("Withdraw completed: %1").arg(info), false);
        m_pendingWithdraws.remove(slateId);
        if (!slateId.isEmpty()) {
            PendingWithdrawConfirmationRecord confirmation{slateId,
                                                         pending.userId,
                                                         message.chat.id,
                                                         message.from.firstName,
                                                         pending.amount,
                                                         QDateTime::currentSecsSinceEpoch()};
            if (!m_db->insertPendingWithdrawConfirmation(confirmation)) {
                qWarning() << "Failed to persist pending withdraw confirmation" << slateId;
            }
            if (!m_db->markPendingWithdrawCompleted(slateId)) {
                qWarning() << "Failed to mark pending withdraw completed" << slateId;
            }
        }
        return true;
    }

    return false;
}

bool TippingWorker::handleSlatepackText(TelegramBotMessage &message, const QString &text)
{
    if (!activateTippingWalletAccount()) {
        sendUserMessage(message, "Wallet account could not be activated.", false);
        return true;
    }

    Slate slate;
    Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(text);
    if (!res.unwrapOrLog(slate)) {
        sendUserMessage(message, QString("Error: %1").arg(res.errorMessage()), false);
        return true;
    }

    SlateState state = Slate::slateStateFromString(slate.sta());
    QString slateId = slate.id();
    if (state == SlateState::I2) {
        PendingDepositRecord pending;
        if (slateId.isEmpty() || !m_db->pendingDeposit(slateId, pending)) {
            qDebug() << "handleSlatepackText: ignoring I2 slate" << slateId << "not pending";
            return false;
        }
        Result<QString> result = handleSlateI2State(slate, message);
        QString info;
        if (!result.unwrapOrLog(info)) {
            sendUserMessage(message, QString("Error: %1").arg(result.errorMessage()), false);
            return true;
        }
        sendUserMessage(message, info, false);
        return true;
    }

    if (state == SlateState::S2) {
        if (slateId.isEmpty() || !m_pendingWithdraws.contains(slateId)) {
            qDebug() << "handleSlatepackText: ignoring S2 slate" << slateId << "not pending withdraw";
            return false;
        }
        PendingWithdrawRecord pending = m_pendingWithdraws.value(slateId);
        Result<QString> result = handleSlateS2State(slate, message, pending);
        QString info;
        if (!result.unwrapOrLog(info)) {
            sendUserMessage(message, QString("Error: %1").arg(result.errorMessage()), false);
            return true;
        }
        sendUserMessage(message, QString("Withdraw completed: %1").arg(info), false);
        m_pendingWithdraws.remove(slateId);
        if (!slateId.isEmpty()) {
            PendingWithdrawConfirmationRecord confirmation{slateId,
                                                         pending.userId,
                                                         message.chat.id,
                                                         message.from.firstName,
                                                         pending.amount,
                                                         QDateTime::currentSecsSinceEpoch()};
            if (!m_db->insertPendingWithdrawConfirmation(confirmation)) {
                qWarning() << "Failed to persist pending withdraw confirmation" << slateId;
            }
            if (!m_db->markPendingWithdrawCompleted(slateId)) {
                qWarning() << "Failed to mark pending withdraw completed" << slateId;
            }
        }
        return true;
    }

    return false;
}

QString TippingWorker::handleDepositCommand(const QString &senderId, int amount, TelegramBotMessage message)
{
    Q_UNUSED(senderId);
    qlonglong nanogrin = qlonglong(amount) * nanogrinPerGrin;
    QString slateId;
    Result<QString> res = createInvoiceSlatepack(nanogrin, slateId);
    QString slatepack;
    if (!res.unwrapOrLog(slatepack)) {
        return QString("Error creating deposit slatepack: %1").arg(res.errorMessage());
    }

    if (!slateId.isEmpty()) {
        PendingDepositRecord pending;
        pending.slateId = slateId;
        pending.userId = senderId;
        pending.chatId = message.chat.id;
        pending.firstName = message.from.firstName;
        pending.amount = nanogrin;
        pending.completed = false;
        if (!m_db->insertPendingDeposit(pending)) {
            qWarning() << "Failed to store pending deposit for slate" << slateId;
        }
        qDebug() << "Deposit command stored pending slate" << slateId << "for" << pending.userId << "amount" << formatGrin(pending.amount);
    sendSlatepackMessage(message, slatepack, "I1", true);
    }
    else
    {
        sendUserMarkdownMessage(message,"Error with slatepack creation (deposit)! Please contact admin",false,true);
    }
    Q_UNUSED(amount);
    return QString();
}

QString TippingWorker::handleWithdrawCommand(const QString &senderId, int amount, TelegramBotMessage message)
{
    qlonglong amountNano = qlonglong(amount) * nanogrinPerGrin;
    if (m_db->getBalance(senderId) < amountNano) {
        return "Insufficient balance.";
    }

    Result<QString> res = createSendSlatepack(amountNano, senderId);
    QString slatepack;
    if (!res.unwrapOrLog(slatepack)) {
        return QString("Error creating withdraw slatepack: %1").arg(res.errorMessage());
    }

    sendSlatepackMessage(message, slatepack, "S1", true);
    Q_UNUSED(amount);
    return QString();
}

QString TippingWorker::handleOpenTransactionsCommand(const QString &sender)
{
    if (!activateTippingWalletAccount()) {
        return "Wallet account could not be activated.";
    }

    QList<TxLogEntry> txList;
    Result<QList<TxLogEntry>> res = m_walletOwnerApi->retrieveTxs(true, 0, "");

    QStringList lines;
    if (!res.unwrapOrLog(txList)) {
        lines << QString("Error fetching open transactions: %1").arg(res.errorMessage());
    } else {
        for (const TxLogEntry &entry : txList) {
            if (entry.confirmed()) {
                continue;
            }

            if (entry.txType() != "TxReceived" && entry.txType() != "TxSent") {
                continue;
            }

            quint64 amount = (entry.txType() == "TxSent") ? entry.amountDebited() : entry.amountCredited();
            QString grinAmount = formatGrin(static_cast<qlonglong>(amount));
            QString slateId = entry.txSlateId().isNull() ? QStringLiteral("-") : entry.txSlateId().toString();
            QString timestamp = entry.creationTs().toString("yyyy-MM-dd hh:mm:ss");
            QString direction = (entry.txType() == "TxSent") ? "Sent" : "Received";
            lines << QString("%1 %2 GRIN (tx id %3, slate %4, created %5)")
                         .arg(direction)
                         .arg(grinAmount)
                         .arg(entry.id())
                         .arg(slateId)
                         .arg(timestamp);
        }
    }

    QList<PendingDepositRecord> pendingList = m_db->pendingDeposits();
    if (!pendingList.isEmpty()) {
        for (const PendingDepositRecord &pending : pendingList) {
            QString name = pending.firstName.isEmpty() ? pending.userId : pending.firstName;
            lines << QString("Pending deposit %1 GRIN for %2 (slate %3)").arg(formatGrin(pending.amount)).arg(name).arg(pending.slateId);
        }
    }

    if (lines.isEmpty()) {
        return "No open transactions found.";
    }

    QString header = QString("Open transactions (%1):").arg(lines.size());
    lines.prepend(header);
    return lines.join("\n");
}

QString TippingWorker::handleLedgerCommand(const QString &senderId, const QString &senderLabel)
{
    Q_UNUSED(senderId);
    Q_UNUSED(senderLabel);

    QList<TxLedgerEntry> entries = m_db->ledgerEntries(20);
    if (entries.isEmpty()) {
        return "No ledger entries available yet.";
    }

    QStringList lines;
    lines << QString("Ledger-Verlauf (letzte %1 Einträge):").arg(entries.size());
    for (const TxLedgerEntry &entry : entries) {
        QDateTime ts = QDateTime::fromSecsSinceEpoch(entry.timestamp);
        QString time = ts.toString("yyyy-MM-dd hh:mm:ss");
        QString from = entry.fromUserId.isEmpty() ? "system" : m_db->usernameByUserId(entry.fromUserId);
        if (from.isEmpty()) {
            from = entry.fromUserId.isEmpty() ? "system" : entry.fromUserId;
        }

        QString to = entry.toUserId.isEmpty() ? "system" : m_db->usernameByUserId(entry.toUserId);
        if (to.isEmpty()) {
            to = entry.toUserId.isEmpty() ? "system" : entry.toUserId;
        }
        QString amount = formatGrin(entry.amount);
        QString line = QString("%1 | %2 GRIN | %3 -> %4 | %5")
                       .arg(time)
                       .arg(amount)
                       .arg(from)
                       .arg(to)
                       .arg(entry.type);
        if (!entry.reference.isEmpty()) {
            line += QString(" (%1)").arg(entry.reference);
        }
        lines << line;
    }

    return lines.join("\n");
}

void TippingWorker::sendSlatepackMessage(TelegramBotMessage message, const QString &slatepack, const QString &stateLabel, bool sendToUserChat)
{
    QString trimmed = slatepack.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    const int chunkSize = 3800;
    int totalParts = (trimmed.size() + chunkSize - 1) / chunkSize;
    int part = 1;
    while (part <= totalParts) {
        QString chunk = trimmed.mid((part - 1) * chunkSize, chunkSize);
        QString header;
        if (totalParts > 1) {
            header = QString("Slatepack (%1) part %2/%3. Copy the text below and send the completed slatepack back when ready.")
                         .arg(stateLabel)
                         .arg(part)
                         .arg(totalParts);
        } else {
            header = QString("Slatepack (%1). Copy the text below and send the completed slatepack back when ready.").arg(stateLabel);
        }

        qDebug() << "sendSlatepackMessage chunk" << part << "of" << totalParts << "size" << chunk.size();

        sendUserMessage(message, header, true, sendToUserChat);
        sendUserMessage(message, chunk, true, sendToUserChat);
        ++part;
    }
}

Result<QString> TippingWorker::createInvoiceSlatepack(qlonglong nanogrin, QString &slateId)
{
    if (!ensureTippingAccount()) {
        return Error(ErrorType::Unknown, "Tipping account could not be activated.");
    }

    qDebug() << "createInvoiceSlatepack: requesting invoice for" << nanogrin << "nanogrin";
    Result<Slate> slateResult = m_walletOwnerApi->issueInvoiceTx(QString::number(nanogrin), "", "");
    Slate slate;
    if (!slateResult.unwrapOrLog(slate)) {
        return slateResult.error();
    }

    slateId = slate.id();
    qDebug() << "createInvoiceSlatepack: created invoice slate" << slateId;

    Result<QString> slatepackResult = m_walletOwnerApi->createSlatepackMessage(slate, QJsonArray(), 0);
    QString slatepack;
    if (slatepackResult.unwrapOrLog(slatepack)) {
        qDebug() << "createInvoiceSlatepack: slatepack size" << slatepack.size();
    }
    return slatepackResult;
}

Result<QString> TippingWorker::createSendSlatepack(qlonglong nanogrin, const QString &senderId)
{
    if (!ensureTippingAccount()) {
        return Error(ErrorType::Unknown, "Tipping account could not be activated.");
    }

    qDebug() << "createSendSlatepack: amount (nanogrin)" << nanogrin << "using account" << tippingAccountLabel;
    InitTxArgs args;
    args.setSrcAcctName(tippingAccountLabel);
    args.setAmount(nanogrin);
    args.setAmountIncludesFee(QJsonValue::Null);
    args.setMinimumConfirmations(10);
    args.setMaxOutputs(500);
    args.setNumChangeOutputs(1);
    args.setSelectionStrategyIsUseAll(false);
    args.setTargetSlateVersion(QJsonValue::Null);
    args.setTtlBlocks(QJsonValue::Null);
    args.setPaymentProofRecipientAddress(QJsonValue::Null);
    args.setEstimateOnly(QJsonValue::Null);
    args.setLateLock(QJsonValue::Null);
    args.setSendArgs(InitTxSendArgs());

    qDebug() << "createSendSlatepack: calling initSendTx with args: minConf=" << args.minimumConfirmations()
             << "maxOutputs=" << args.maxOutputs()
             << "selectionUseAll=" << args.selectionStrategyIsUseAll();
    Result<Slate> slateResult = m_walletOwnerApi->initSendTx(args);
    Slate slate;
    if (!slateResult.unwrapOrLog(slate)) {
        return slateResult.error();
    }

    {
        Result<bool> lockResult = m_walletOwnerApi->txLockOutputs(slate);
        bool locked = false;
        if (!lockResult.unwrapOrLog(locked)) {
            return lockResult.error();
        }
        qDebug() << "createSendSlatepack: txLockOutputs" << locked;
    }

    QString slateId = slate.id();
    if (!slateId.isEmpty()) {
        PendingWithdrawRecord pending{slateId, senderId, nanogrin, QDateTime::currentSecsSinceEpoch()};
        m_pendingWithdraws.insert(slateId, pending);
        if (!m_db->insertPendingWithdraw(pending)) {
            qWarning() << "createSendSlatepack: failed to persist pending withdraw" << slateId;
        }
        qDebug() << "createSendSlatepack: stored pending withdraw" << slateId << "amount" << formatGrin(nanogrin);
    }

    Result<QString> slatepackResult = m_walletOwnerApi->createSlatepackMessage(slate, QJsonArray(), 0);
    QString slatePackMessage;
    if (slatepackResult.unwrapOrLog(slatePackMessage)) {
        qDebug() << "createSendSlatepack: slatepack length" << slatePackMessage.length();
    }
    return slatepackResult;
}

Result<QString> TippingWorker::handleSlateI2State(Slate slate, TelegramBotMessage message)
{
    if (!activateTippingWalletAccount()) {
        return Error(ErrorType::Unknown, "Wallet account could not be activated.");
    }

    Slate finalized;
    {
        qDebug() << "handleSlateI2State: finalizing deposit slate for" << message.from.firstName;
        Result<Slate> res = m_walletOwnerApi->finalizeTx(slate);
        if (!res.unwrapOrLog(finalized)) {
            return res.error();
        }
        qDebug() << "handleSlateI2State: finalizeTx successful, slate amount" << finalized.amt();
    }

    {
        bool posted = false;
        Result<bool> res = m_walletOwnerApi->postTx(finalized, false);
        if (!res.unwrapOrLog(posted)) {
            QString err = res.errorMessage();
            if (err.contains("no result element", Qt::CaseInsensitive)) {
                qWarning() << "handleSlateI2State: postTx reported missing result element";
                qWarning() << "postTx returned missing result element; proceeding as success";
            } else {
                return res.error();
            }
        }
        qDebug() << "handleSlateI2State: postTx completed (posted=" << posted << ")";
    }

    QString slateId = finalized.id();
    qlonglong nanogrin = slateToGrin(finalized);

    PendingDepositRecord pending;
    bool hasPending = false;
    if (!slateId.isEmpty()) {
        hasPending = m_db->pendingDeposit(slateId, pending);
    }

    qlonglong displayAmount = hasPending ? pending.amount : nanogrin;

    if (displayAmount <= 0) {
        return Error(ErrorType::Unknown, "Deposit amount missing.");
    }

    return QString("%1 GRIN deposit received. Waiting for confirmations before crediting.").arg(formatGrin(displayAmount));
}

Result<QString> TippingWorker::handleSlateS2State(Slate slate, TelegramBotMessage message, const PendingWithdrawRecord &pendingWithdraw)
{
    if (!ensureTippingAccount()) {
        return Error(ErrorType::Unknown, "Tipping account not available.");
    }

    Slate finalized;
    {
        qDebug() << "handleSlateS2State: finalizing withdraw slate for" << message.from.firstName;
        Result<Slate> res = m_walletOwnerApi->finalizeTx(slate);
        if (!res.unwrapOrLog(finalized)) {
            qDebug()<<"error finalize!";
            return res.error();
        }
        qDebug() << "handleSlateS2State: finalizeTx success, slate amount" << finalized.amt();
    }

    {
        bool posted = false;
        Result<bool> res = m_walletOwnerApi->postTx(finalized, false);
        if (!res.unwrapOrLog(posted)) {
            return res.error();
        }
        qDebug() << "handleSlateS2State: postTx completed (posted=" << posted << ")";
    }

    qlonglong nanogrin = slateToGrin(finalized);
    qlonglong grinAmount = pendingWithdraw.amount;
    if (grinAmount <= 0) {
        grinAmount = nanogrin;
    }
    QString userId = pendingWithdraw.userId.isEmpty() ? userLabel(message) : pendingWithdraw.userId;
    if (!m_db->updateBalance(userId, -grinAmount)) {
        return Error(ErrorType::Unknown, "Error deducting from account.");
    }
    m_db->recordTransaction(userId, "", grinAmount, "withdraw");

    return QString("%1 GRIN withdrawn.").arg(formatGrin(grinAmount));
}

qlonglong TippingWorker::slateToGrin(const Slate &slate) const
{
    bool ok;
    qlonglong value = slate.amt().toLongLong(&ok);
    if (!ok) {
        return 0;
    }
    return value;
}

bool TippingWorker::ensureTippingAccount()
{
    Result<QList<Account>> res = m_walletOwnerApi->accounts();
    QList<Account> accounts;
    if (!res.unwrapOrLog(accounts)) {
        return false;
    }

    bool found = false;
    for (const Account &account : accounts) {
        if (account.label() == tippingAccountLabel) {
            found = true;
            break;
        }
    }

    if (!found) {
        Result<QString> createRes = m_walletOwnerApi->createAccountPath(tippingAccountLabel);
        QString path;
        if (!createRes.unwrapOrLog(path)) {
            return false;
        }
    }

    return activateTippingWalletAccount();
}

Result<WalletInfo> TippingWorker::fetchAccountSummary(const QString &accountLabel)
{
    Result<bool> setRes = m_walletOwnerApi->setActiveAccount(accountLabel);
    bool activated = false;
    if (!setRes.unwrapOrLog(activated)) {
        return Error(ErrorType::Unknown, setRes.errorMessage());
    }
    if (!activated) {
        return Error(ErrorType::Unknown, QString("Failed to activate account %1").arg(accountLabel));
    }

    return m_walletOwnerApi->retrieveSummaryInfo(true, 1);
}

bool TippingWorker::activateWalletAccount(const QString &accountLabel)
{
    if (!m_walletOwnerApi) {
        qWarning() << "Wallet owner API is not initialized";
        return false;
    }

    Result<bool> res = m_walletOwnerApi->setActiveAccount(accountLabel);
    bool activated = false;
    if (!res.unwrapOrLog(activated)) {
        qWarning() << "Failed to activate wallet account" << accountLabel << ":" << res.errorMessage();
        return false;
    }

    if (!activated) {
        qWarning() << "Wallet account activation returned false for" << accountLabel;
    }

    return activated;
}

bool TippingWorker::activateTippingWalletAccount()
{
    return activateWalletAccount(tippingAccountLabel);
}

QString TippingWorker::formatWalletSummary(const WalletInfo &walletInfo) const
{
    auto toGrin = [](quint64 nano) {
        return static_cast<qlonglong>(nano / 1000000000ULL);
    };

    QStringList lines;
    lines << QString("amountAwaitingConfirmation: %1 GRIN").arg(toGrin(walletInfo.amountAwaitingConfirmation()));
    lines << QString("amountAwaitingFinalization: %1 GRIN").arg(toGrin(walletInfo.amountAwaitingFinalization()));
    lines << QString("amountCurrentlySpendable: %1 GRIN").arg(toGrin(walletInfo.amountCurrentlySpendable()));
    lines << QString("amountImmature: %1 GRIN").arg(toGrin(walletInfo.amountImmature()));
    lines << QString("amountLocked: %1 GRIN").arg(toGrin(walletInfo.amountLocked()));
    lines << QString("amountReverted: %1 GRIN").arg(toGrin(walletInfo.amountReverted()));
    lines << QString("total: %1 GRIN").arg(toGrin(walletInfo.total()));
    lines << QString("lastConfirmedHeight: %1").arg(walletInfo.lastConfirmedHeight());
    lines << QString("minimumConfirmations: %1").arg(walletInfo.minimumConfirmations());
    return lines.join("\n");
}

QString TippingWorker::handleAdminAmountsCommand()
{
    QString info;
    WalletInfo walletInfo;
    Result<WalletInfo> tippingSummary = fetchAccountSummary(tippingAccountLabel);
    if (!tippingSummary.unwrapOrLog(walletInfo)) {
        QString err = tippingSummary.errorMessage();
        if (err.contains("no result element", Qt::CaseInsensitive)) {
            info.append("Tipping summary: Wallet information not available yet. Please try again shortly.\n\n");
        } else {
            info.append(QString("Tipping summary error: %1\n\n").arg(err));
        }
    } else {
        info.append("Tipping summary:\n" + formatWalletSummary(walletInfo) + "\n\n");
    }

    QList<Account> accounts;
    Result<QList<Account>> accountsRes = m_walletOwnerApi->accounts();
    if (accountsRes.unwrapOrLog(accounts)) {
        QString ggcLabel;
        for (const Account &account : accounts) {
            if (account.label() != tippingAccountLabel) {
                ggcLabel = account.label();
                break;
            }
        }

        if (!ggcLabel.isEmpty()) {
            WalletInfo ggcInfo;
            Result<WalletInfo> ggcSummary = fetchAccountSummary(ggcLabel);
            if (!ggcSummary.unwrapOrLog(ggcInfo)) {
                info.append(QString("GGC summary error: %1\n").arg(ggcSummary.errorMessage()));
            } else {
                info.append("GGC summary:\n" + formatWalletSummary(ggcInfo));
            }
            m_walletOwnerApi->setActiveAccount(tippingAccountLabel);
        } else {
            info.append("GGC summary: no additional account found.\n");
        }
    } else {
        info.append("GGC summary: unable to list accounts.\n");
    }

    return info.trimmed();
}

bool TippingWorker::isAdmin(qlonglong id)
{
    QStringList stringList = m_settings->value("admin/telegramIds").toStringList();

    for (const QString &str : stringList) {
        bool ok;
        qlonglong num = str.toLongLong(&ok);
        if (ok) {
            if (num == id) {
                return true;
            }
        } else {
            qWarning() << "error admin entry:" << str;
        }
    }
    return false;
}

void TippingWorker::checkPendingDeposits()
{
    if (!activateTippingWalletAccount()) {
        qWarning() << "checkPendingDeposits: wallet account could not be activated";
        return;
    }

    QList<PendingDepositRecord> pendingList = m_db->pendingDeposits();
    if (pendingList.isEmpty()) {
        return;
    }

    qDebug() << "checkPendingDeposits: found" << pendingList.size() << "pending entries";
    for (const PendingDepositRecord &pending : pendingList) {
        const QString &slateId = pending.slateId;

        qDebug() << "checkPendingDeposits: querying txs for slate" << slateId;
        QList<TxLogEntry> txList;
        Result<QList<TxLogEntry>> res = m_walletOwnerApi->retrieveTxs(true, 0, "");
        if (!res.unwrapOrLog(txList)) {
            qDebug() << "checkPendingDeposits: retrieveTxs failed for" << slateId << "-" << res.errorMessage();
            continue;
        }

        for (const TxLogEntry &entry : txList) {
            if (entry.txSlateId().isNull()) {
                qDebug()<<"entry.txSlateId().isNull()";
                continue;
            }

            QString strSlate = entry.txSlateId().toString().replace("{","").replace("}","");
            if (strSlate != slateId) {
                continue;
            }

            if (!entry.confirmed()) {
                qDebug() << "checkPendingDeposits: tx" << entry.id() << "not confirmed yet";
                continue;
            }

            qlonglong creditedAmount = pending.amount;
            quint64 amountNano = entry.amountCredited() > 0 ? entry.amountCredited() : entry.amountDebited();
            if (amountNano > 0) {
                creditedAmount = static_cast<qlonglong>(amountNano);
            }
            if (creditedAmount <= 0) {
                creditedAmount = pending.amount;
            }

            qDebug() << "checkPendingDeposits: crediting" << creditedAmount << "GRIN to" << pending.userId << "for tx" << entry.id();
            m_db->updateBalance(pending.userId, creditedAmount);
            m_db->recordTransaction("", pending.userId, creditedAmount, "deposit");

            QString reply = QString("%1 GRIN deposit confirmed and credited to your balance.").arg(formatGrin(creditedAmount));
            QString msg;
            if (!pending.firstName.isEmpty()) {
                msg = QString("Hi %1,\n%2").arg(pending.firstName).arg(reply);
            } else {
                msg = reply;
            }
            sendUserDirectMessage(pending.userId, msg, true);

            if (!m_db->markPendingDepositCompleted(slateId)) {
                qWarning() << "Failed to mark pending deposit completed" << slateId;
            } else {
                qDebug() << "checkPendingDeposits: marked pending deposit completed" << slateId;
            }
            break;
        }
    }
}

void TippingWorker::checkPendingWithdrawConfirmations()
{
    if (!activateTippingWalletAccount()) {
        qWarning() << "checkPendingWithdrawConfirmations: wallet account could not be activated";
        return;
    }

    QList<PendingWithdrawConfirmationRecord> pendingList = m_db->pendingWithdrawConfirmations();
    if (pendingList.isEmpty()) {
        return;
    }

    QList<TxLogEntry> txList;
    Result<QList<TxLogEntry>> res = m_walletOwnerApi->retrieveTxs(true, 0, "");
    if (!res.unwrapOrLog(txList)) {
        qWarning() << "checkPendingWithdrawConfirmations: retrieveTxs failed -" << res.errorMessage();
        return;
    }

    for (const PendingWithdrawConfirmationRecord &pending : pendingList) {
        qDebug()<<"pendingList:  "<<pending.firstName<<"   "<<pending.slateId;
        for (const TxLogEntry &entry : txList) {
            if (entry.txSlateId().isNull()) {
                continue;
            }

            QString strSlate = entry.txSlateId().toString().replace("{", "").replace("}", "");
            if (strSlate != pending.slateId) {
                continue;
            }

            if (!entry.confirmed()) {
                qDebug() << "checkPendingWithdrawConfirmations: tx" << entry.id() << "not confirmed yet for slate" << pending.slateId;
                continue;
            }

            QString reply = QString("%1 GRIN withdraw confirmed on the blockchain (tx id %2).")
                            .arg(formatGrin(pending.amount))
                            .arg(entry.id());
            QString msg;
            if (!pending.firstName.isEmpty()) {
                msg = QString("Hi %1,\n%2").arg(pending.firstName).arg(reply);
            } else {
                msg = reply;
            }
            sendUserDirectMessage(pending.userId, msg, true);

            if (!m_db->markPendingWithdrawConfirmationCompleted(pending.slateId)) {
                qWarning() << "Failed to mark pending withdraw confirmation" << pending.slateId;
            }
            break;
        }
    }
}

QString TippingWorker::userLabel(const TelegramBotMessage &message) const
{
    if (!message.from.username.isEmpty()) {
        return message.from.username;
    }
    return QString::number(message.from.id);
}

QString TippingWorker::downloadFileToQString(const QUrl &url)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(url);

    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply->readAll();
        result = QString::fromUtf8(bytes);
    } else {
        qWarning() << "Download error:" << reply->errorString();
    }

    reply->deleteLater();
    return result;
}

void TippingWorker::sendUserMessage(QString user, QString content)
{
    Q_UNUSED(user);
    qDebug() << "Message to " << user << ": " << content;
}

void TippingWorker::sendUserDirectMessage(const QString &userId, QString content, bool plain)
{
    if (userId.isEmpty()) {
        return;
    }

    bool ok = false;
    qlonglong chatId = userId.toLongLong(&ok);
    if (!ok) {
        qWarning() << "Invalid userId for direct message:" << userId;
        return;
    }

    QVariant chatVar = QVariant::fromValue(chatId);
    m_bot->sendMessage(chatVar,
                       content,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}

void TippingWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain, bool sendToUserChat)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi " + message.from.firstName + ",\n" + content);
    }

    QVariant chatId = sendToUserChat ? QVariant::fromValue(message.from.id) : QVariant::fromValue(message.chat.id);
    m_bot->sendMessage(chatId,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}

QString TippingWorker::readFileToString(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open tipping guide:" << file.errorString();
        return {};
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

void TippingWorker::sendUserMarkdownMessage(TelegramBotMessage message, QString content, bool plain, bool sendToUserChat)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi " + message.from.firstName + ",\n" + content);
    }

    QVariant chatId = sendToUserChat ? QVariant::fromValue(message.from.id) : QVariant::fromValue(message.chat.id);
    m_bot->sendMessage(chatId,
                       msg,
                       0,
                       TelegramBot::Markdown | TelegramBot::DisableWebPagePreview,
                       TelegramKeyboardRequest(),
                       nullptr);
}

QString TippingWorker::resolveRecipientId(const QString &target, const TelegramBotMessage &message) const
{
    QString normalized = target.trimmed();
    if (normalized.startsWith("@")) {
        normalized = normalized.mid(1);
    }

    if (normalized.isEmpty()) {
        for (const TelegramBotMessageEntity &entity : message.entities) {
            if (entity.type == "text_mention" && entity.user.id != 0) {
                return QString::number(entity.user.id);
            }
        }
        return {};
    }

    bool ok = false;
    qlonglong numericId = normalized.toLongLong(&ok);
    if (ok) {
        return normalized;
    }

    QString foundId = m_db->userIdByUsername(normalized);
    if (!foundId.isEmpty()) {
        return foundId;
    }

    for (const TelegramBotMessageEntity &entity : message.entities) {
        if (entity.type == "text_mention" && entity.user.id != 0) {
            QString entityText = message.text.mid(entity.offset, entity.length);
            QString stripped = entityText.trimmed();
            if (stripped.startsWith("@")) {
                stripped = stripped.mid(1);
            }
            if (!stripped.isEmpty() && stripped.compare(normalized, Qt::CaseInsensitive) == 0) {
                return QString::number(entity.user.id);
            }
        }
    }

    return {};
}
