#include "tippingworker.h"
#include <QByteArray>
#include <QList>

#include <QtGlobal>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QVariant>
#include <QJsonArray>
#include <QStringList>
#include <QHash>
#include <cmath>
#include "txlogentry.h"

namespace {
//fix var
constexpr qlonglong nanogrinPerGrin = 1000000000LL;


/**
 * @brief requiredBotMention
 * @return
 */
QString requiredBotMention()
{
    return qEnvironmentVariable("GRIN_CHAIN_TYPE") == "testnet" ? "@grin_mw_test_bot" : "@grin_mw_bot";
}

/**
 * @brief normalizeCommandText
 * @param text
 * @return
 */
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

/**
 * @brief formatGrin
 * @param nanogrin
 * @return
 */
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

/**
 * @brief parseTipAmount
 * @param input
 * @param nanogrin
 * @param errorMessage
 * @return
 */
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

/**
 * @brief canonicalizeBipPath
 * @param value
 * @return
 */
QString canonicalizeBipPath(const QString &value)
{
    QString normalized = value.trimmed().toLower();
    if (normalized.isEmpty() || normalized == QStringLiteral("m")) {
        return QStringLiteral("m");
    }
    if (normalized.startsWith(QStringLiteral("m/"))) {
        return normalized;
    }
    return QStringLiteral("m/") + normalized;
}

/**
 * @brief parseAccountPathHex
 * @param hexPath
 * @return
 */
QString parseAccountPathHex(const QString &hexPath)
{
    QString trimmed = hexPath.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    QByteArray data = QByteArray::fromHex(trimmed.toLatin1());
    if (data.isEmpty()) {
        return QString();
    }

    quint8 depth = static_cast<quint8>(data[0]);
    if (depth == 0 || data.size() < 1) {
        return QStringLiteral("m");
    }

    if (data.size() < 1 + static_cast<int>(depth) * 4) {
        return QString();
    }

    QString result = QStringLiteral("m");
    int offset = 1;
    for (int i = 0; i < depth; ++i) {
        if (offset + 4 > data.size()) {
            return QString();
        }

        quint32 index = (static_cast<quint32>(static_cast<uchar>(data[offset])) << 24) |
                        (static_cast<quint32>(static_cast<uchar>(data[offset + 1])) << 16) |
                        (static_cast<quint32>(static_cast<uchar>(data[offset + 2])) << 8) |
                        static_cast<quint32>(static_cast<uchar>(data[offset + 3]));
        offset += 4;
        result += QStringLiteral("/") + QString::number(index);
    }

    return result;
}
}

/**
 * @brief normalizedSlateId
 * @param entry
 * @return
 */
QString normalizedSlateId(const TxLogEntry &entry)
{
    if (entry.txSlateId().isNull()) {
        return {};
    }
    QString slateId = entry.txSlateId().toString();
    return slateId.remove('{').remove('}');
}

/**
 * @brief TippingWorker::TippingWorker
 * @param bot
 * @param settings
 * @param walletOwnerApi
 */
TippingWorker::TippingWorker(TelegramBot *bot, QSettings *settings, WalletOwnerApi *walletOwnerApi) :
    m_bot(bot),
    m_settings(settings),
    m_db(nullptr),
    m_walletOwnerApi(walletOwnerApi),
    m_tippingAccountLabel(),
    m_tippingAccountPath(),
    walletPassword("test"),
    m_pendingDepositTimer(nullptr)
{
}

/**
 * @brief TippingWorker::init
 * @return
 */
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

    if (!m_walletOwnerApi) {
        qWarning() << "Wallet owner API is not initialized";
        return false;
    }

    if (!resolveTippingAccountLabel()) {
        qWarning() << "Failed to resolve tipping account label from settings.";
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

/**
 * @brief TippingWorker::handleUpdate
 * @param update
 * @return
 */
bool TippingWorker::handleUpdate(TelegramBotUpdate update)
{

    //-----------------------------------------------------------------------------------------------------------------------------------
    // check parameter
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (!update || update.isNull() || update->type != TelegramBotMessageType::Message) {
        return false;
    }


    TelegramBotMessage &message = *update->message;
    QString text = normalizeCommandText(message.text);
    const QString senderId = QString::number(message.from.id);
    const bool isPrivateChat = message.chat.id > 0;
    const bool knownPrivateChat = !senderId.isEmpty() && m_db && m_db->userHasPrivateChat(senderId);

    const QString username = message.from.username;
    const QString firstName = message.from.firstName;

    if (message.chat.id < 0 && !knownPrivateChat) {
        sendUserMessage(message,
                        "Please send me a private message and press start once so I can reply to you directly in the future and avoid cluttering the group chat.",
                        false,
                        false);
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (!senderId.isEmpty() && (!username.isEmpty() || !firstName.isEmpty())) {
        qDebug()<<"ensure user Record: "<<m_db->ensureUserRecord(senderId, username, firstName, isPrivateChat);
    }


    if (text.isEmpty()) {
        return false;
    }

    if (!message.document.fileId.isEmpty() && !message.document.fileName.isEmpty()) {
        return handleSlatepackDocument(message);
    }


    if (text.contains("BEGINSLATEPACK") && text.contains("ENDSLATEPACK")) {
        return handleSlatepackText(message, text);
    }

    const QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return false;

    const QString cmd = parts[0];
    const QString sender = userLabel(message);

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/adminamounts") {
        if (!isAdmin(message.from.id)) {
            sendUserDirectMessage(senderId, "You are not authorized to run this command.", false, &message);
            return true;
        }
        sendUserDirectMessage(senderId, handleAdminAmountsCommand(), false, &message);
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/adminbalance") {
        if (!isAdmin(message.from.id)) {
            sendUserDirectMessage(senderId, "You are not authorized to run this command.", false, &message);
            return true;
        }
        QList<BalanceRecord> balances = m_db->listBalances();
        if (balances.isEmpty()) {
            sendUserDirectMessage(senderId, "No balances available yet.", false, &message);
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
        sendUserDirectMessage(senderId, lines.join("\n"), false, &message);
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/deposit") {
        if (parts.size() != 2) {
            sendUserDirectMessage(senderId, "Usage: /deposit <amount in GRIN>", false, &message);
            return true;
        }
        bool ok;
        int amount = parts[1].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserDirectMessage(senderId, "Please provide a positive amount.", false, &message);
            return true;
        }
        QString depositResponse = handleDepositCommand(senderId, amount, message);
        if (!depositResponse.isEmpty()) {
            sendUserDirectMessage(senderId, depositResponse, false, &message);
        }
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/withdraw") {
        if (parts.size() != 2) {
            sendUserDirectMessage(senderId, "Usage: /withdraw <amount in GRIN>", false, &message);
            return true;
        }
        bool ok;
        int amount = parts[1].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserDirectMessage(senderId, "Please provide a positive amount.", false, &message);
            return true;
        }
        QString withdrawResponse = handleWithdrawCommand(senderId, amount, message);
        if (!withdrawResponse.isEmpty()) {
            sendUserDirectMessage(senderId, withdrawResponse, false, &message);
        }
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/tip") {
        if (parts.size() != 3) {
            sendUserMessage(message, "Usage: /tip @user <amount>", false);
            return true;
        }
        QString toUser = parts[1];
        qlonglong tipNanogrin = 0;
        QString tipError;
        if (!parseTipAmount(parts[2], tipNanogrin, tipError)) {
            sendUserMessage(message, tipError, false);
            return true;
        }

        QString recipientId = resolveRecipientId(toUser, message);

        qDebug()<<"recipientId: "<<recipientId;

        if (recipientId.isEmpty()) {
            sendUserMessage(message,
                            "I could not resolve that recipient. Ask them to send me a private /tipping message so I can learn their ID, then try again.",
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

        qDebug()<<"m_db->getBalance(senderId): "<<m_db->getBalance(senderId)<<"  <  "<<tipNanogrin;
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

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/balance") {
        qlonglong bal = m_db->getBalance(senderId);
        sendUserDirectMessage(senderId, QString("Your current balance: %1 GRIN").arg(formatGrin(bal)), false, &message);
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/ledger") {
        sendUserDirectMessage(senderId, handleLedgerCommand(senderId, sender), false, &message);
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //
    //-----------------------------------------------------------------------------------------------------------------------------------
    if (cmd == "/opentxs") {
            sendUserDirectMessage(senderId, handleOpenTransactionsCommand(sender), false, &message);
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
    if (!res.unwrapOrLog(slate, Q_FUNC_INFO)) {
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
        if (!result.unwrapOrLog(info, Q_FUNC_INFO)) {
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
        if (!result.unwrapOrLog(info, Q_FUNC_INFO)) {
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
    if (!res.unwrapOrLog(slate, Q_FUNC_INFO)) {
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
        if (!result.unwrapOrLog(info, Q_FUNC_INFO)) {
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
        if (!result.unwrapOrLog(info, Q_FUNC_INFO)) {
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
    if (!res.unwrapOrLog(slatepack, Q_FUNC_INFO)) {
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
    if (!res.unwrapOrLog(slatepack, Q_FUNC_INFO)) {
        return QString("Error creating withdraw slatepack: %1").arg(res.errorMessage());
    }

    sendSlatepackMessage(message, slatepack, "S1", true);
    Q_UNUSED(amount);
    return QString();
}

QString TippingWorker::handleOpenTransactionsCommand(const QString &sender)
{
    Q_UNUSED(sender);
    if (!activateTippingWalletAccount()) {
        return "Wallet account could not be activated.";
    }

    QList<TxLogEntry> txList;
    Result<QList<TxLogEntry>> res = m_walletOwnerApi->retrieveTxs(true, 0, "");

    QStringList lines;
    if (!res.unwrapOrLog(txList, Q_FUNC_INFO)) {
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
    if (!slateResult.unwrapOrLog(slate, Q_FUNC_INFO)) {
        return slateResult.error();
    }

    slateId = slate.id();
    qDebug() << "createInvoiceSlatepack: created invoice slate" << slateId;

    Result<QString> slatepackResult = m_walletOwnerApi->createSlatepackMessage(slate, QJsonArray(), 0);
    QString slatepack;
    if (slatepackResult.unwrapOrLog(slatepack, Q_FUNC_INFO)) {
        qDebug() << "createInvoiceSlatepack: slatepack size" << slatepack.size();
    }
    return slatepackResult;
}

Result<QString> TippingWorker::createSendSlatepack(qlonglong nanogrin, const QString &senderId)
{
    if (!ensureTippingAccount()) {
        return Error(ErrorType::Unknown, "Tipping account could not be activated.");
    }

    qDebug() << "createSendSlatepack: amount (nanogrin)" << nanogrin << "using account" << m_tippingAccountLabel;
    InitTxArgs args;
    args.setSrcAcctName(m_tippingAccountLabel);
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
    if (!slateResult.unwrapOrLog(slate, Q_FUNC_INFO)) {
        return slateResult.error();
    }

    {
        Result<bool> lockResult = m_walletOwnerApi->txLockOutputs(slate);
        bool locked = false;
        if (!lockResult.unwrapOrLog(locked, Q_FUNC_INFO)) {
            return lockResult.error();
        }
        qDebug() << "createSendSlatepack: txLockOutputs" << locked;
    }

    QString slateId = slate.id();
    if (!slateId.isEmpty()) {
        if (!m_db->updateBalance(senderId, -nanogrin)) {
            qWarning() << "createSendSlatepack: balance reservation failed for" << senderId << "amount" << formatGrin(nanogrin);
            return Error(ErrorType::Unknown, "Unable to reserve balance for withdraw.");
        }

        PendingWithdrawRecord pending{slateId, senderId, nanogrin, QDateTime::currentSecsSinceEpoch()};
        if (!m_db->insertPendingWithdraw(pending)) {
            qWarning() << "createSendSlatepack: failed to persist pending withdraw" << slateId;
            m_db->updateBalance(senderId, nanogrin);
            return Error(ErrorType::Unknown, "Unable to persist withdraw state.");
        }

        m_pendingWithdraws.insert(slateId, pending);
        m_db->recordTransaction(senderId, "", nanogrin, "withdraw_request");
        qDebug() << "createSendSlatepack: stored pending withdraw" << slateId << "amount" << formatGrin(nanogrin);
    }

    Result<QString> slatepackResult = m_walletOwnerApi->createSlatepackMessage(slate, QJsonArray(), 0);
    QString slatePackMessage;
    if (slatepackResult.unwrapOrLog(slatePackMessage, Q_FUNC_INFO)) {
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
        if (!res.unwrapOrLog(finalized, Q_FUNC_INFO)) {
            return res.error();
        }
        qDebug() << "handleSlateI2State: finalizeTx successful, slate amount" << finalized.amt();
    }

    {
        bool posted = false;
        Result<bool> res = m_walletOwnerApi->postTx(finalized, false);
        if (!res.unwrapOrLog(posted, Q_FUNC_INFO)) {
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
        if (!res.unwrapOrLog(finalized, Q_FUNC_INFO)) {
            qDebug()<<"error finalize!";
            return res.error();
        }
        qDebug() << "handleSlateS2State: finalizeTx success, slate amount" << finalized.amt();
    }

    {
        bool posted = false;
        Result<bool> res = m_walletOwnerApi->postTx(finalized, false);
        if (!res.unwrapOrLog(posted, Q_FUNC_INFO)) {
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
    if (m_tippingAccountLabel.isEmpty()) {
        qWarning() << "TippingWorker: account label is not configured";
        return false;
    }

    return activateWalletAccount(m_tippingAccountLabel);
}

bool TippingWorker::resolveTippingAccountLabel()
{
    if (!m_settings) {
        qWarning() << "TippingWorker: settings not available";
        return false;
    }

    m_tippingAccountPath = m_settings->value("wallet/tippingAccountPath").toString().trimmed();
    if (m_tippingAccountPath.isEmpty()) {
        m_tippingAccountPath = "m/1/0";
    }

    Result<QList<Account>> accountsRes = m_walletOwnerApi->accounts();
    QList<Account> accounts;
    if (!accountsRes.unwrapOrLog(accounts, Q_FUNC_INFO)) {
        qWarning() << "TippingWorker: failed to list wallet accounts:" << accountsRes.errorMessage();
        return false;
    }

    QString targetPath = canonicalizeBipPath(m_tippingAccountPath);
    qDebug() << "TippingWorker: searching for canonical path" << targetPath << "raw" << m_tippingAccountPath;
    for (const Account &account : accounts) {
        QString accountBipPath = parseAccountPathHex(account.path());
        QString normalizedAccountPath = accountBipPath.isEmpty() ? QString() : canonicalizeBipPath(accountBipPath);
        qDebug() << "TippingWorker account" << account.label() << "raw" << account.path()
                 << "bip" << accountBipPath << "normalized" << normalizedAccountPath;
        if (!normalizedAccountPath.isEmpty() && !account.label().isEmpty() && normalizedAccountPath == targetPath) {
            m_tippingAccountLabel = account.label();
            qDebug() << "TippingWorker: using label" << m_tippingAccountLabel << "for path" << m_tippingAccountPath;
            return true;
        }
    }

    qWarning() << "TippingWorker: no account matches path" << m_tippingAccountPath;
    return false;
}

Result<WalletInfo> TippingWorker::fetchAccountSummary(const QString &accountLabel)
{
    Result<bool> setRes = m_walletOwnerApi->setActiveAccount(accountLabel);
    bool activated = false;
    if (!setRes.unwrapOrLog(activated, Q_FUNC_INFO)) {
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
    if (!res.unwrapOrLog(activated, Q_FUNC_INFO)) {
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
    return activateWalletAccount(m_tippingAccountLabel);
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
    QList<Account> accounts;
    Result<QList<Account>> accountsRes = m_walletOwnerApi->accounts();
    if (!accountsRes.unwrapOrLog(accounts, Q_FUNC_INFO)) {
        return "Unable to list wallet accounts.";
    }

    for (const Account &account : accounts) {
        if (account.label().isEmpty()) {
            continue;
        }

        WalletInfo summary;
        Result<WalletInfo> accountSummary = fetchAccountSummary(account.label());
        if (!accountSummary.unwrapOrLog(summary, Q_FUNC_INFO)) {
            info.append(QString("Account %1 (%2) summary error: %3\n\n")
                        .arg(account.label(), account.path(), accountSummary.errorMessage()));
        } else {
            info.append(QString("Account %1 (%2) summary:\n%3\n\n")
                        .arg(account.label(), account.path(), formatWalletSummary(summary)));
        }
    }

    if (!m_tippingAccountLabel.isEmpty()) {
        m_walletOwnerApi->setActiveAccount(m_tippingAccountLabel);
    }

    if (info.trimmed().isEmpty()) {
        return "No wallet account summary available.";
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

    QList<TxLogEntry> txList;
    Result<QList<TxLogEntry>> res = m_walletOwnerApi->retrieveTxs(true, 0, "");
    if (!res.unwrapOrLog(txList, Q_FUNC_INFO)) {
        qDebug() << "checkPendingDeposits: retrieveTxs failed -" << res.errorMessage();
        return;
    }

    QHash<QString, TxLogEntry> txBySlate;
    for (const TxLogEntry &entry : txList) {
        QString slateId = normalizedSlateId(entry);
        if (slateId.isEmpty()) {
            continue;
        }
        txBySlate.insert(slateId, entry);
    }

    for (const PendingDepositRecord &pending : pendingList) {
        const QString &slateId = pending.slateId;

        if (slateId.isEmpty()) {
            continue;
        }

        if (!txBySlate.contains(slateId)) {
            qDebug() << "checkPendingDeposits: slate" << slateId << "not found; assuming cleanup removed tx";
            if (!m_db->markPendingDepositCompleted(slateId)) {
                qWarning() << "Failed to mark missing pending deposit completed" << slateId;
            }
            QString notice = QString("Hi %1,\ndeine Deposit-Transaktion (%2) wurde vom Wallet zurückgezogen. Bitte sende erneut.")
                                 .arg(pending.firstName.isEmpty() ? pending.userId : pending.firstName)
                                 .arg(formatGrin(pending.amount));
            sendUserDirectMessage(pending.userId, notice, true, nullptr);
            continue;
        }

        const TxLogEntry entry = txBySlate.value(slateId);
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
        sendUserDirectMessage(pending.userId, msg, true, nullptr);

        if (!m_db->markPendingDepositCompleted(slateId)) {
            qWarning() << "Failed to mark pending deposit completed" << slateId;
        } else {
            qDebug() << "checkPendingDeposits: marked pending deposit completed" << slateId;
        }
    }
}

void TippingWorker::checkPendingWithdrawConfirmations()
{
    if (!activateTippingWalletAccount()) {
        qWarning() << "checkPendingWithdrawConfirmations: wallet account could not be activated";
        return;
    }

    QList<TxLogEntry> txList;
    Result<QList<TxLogEntry>> res = m_walletOwnerApi->retrieveTxs(true, 0, "");
    if (!res.unwrapOrLog(txList, Q_FUNC_INFO)) {
        qWarning() << "checkPendingWithdrawConfirmations: retrieveTxs failed -" << res.errorMessage();
        return;
    }

    QHash<QString, TxLogEntry> txBySlate;
    for (const TxLogEntry &entry : txList) {
        QString slateId = normalizedSlateId(entry);
        if (slateId.isEmpty()) {
            continue;
        }
        txBySlate.insert(slateId, entry);
    }

    QList<PendingWithdrawRecord> withdrawList = m_db->pendingWithdrawals();

    const QString cancellationReason = "was pulled back by the wallet. The amount has been re-credited to your balance.";
    for (const PendingWithdrawRecord &pending : withdrawList) {
        if (pending.slateId.isEmpty()) {
            continue;
        }

        if (!txBySlate.contains(pending.slateId)) {
            continue;
        }

        const TxLogEntry &entry = txBySlate.value(pending.slateId);
        QString type = entry.txType();
        if (type == "TxSentCancelled" || type == "TxReceivedCancelled") {
            qDebug() << "checkPendingWithdrawConfirmations: withdraw slate" << pending.slateId << "was cancelled via tx log";
            if (pending.amount > 0 && m_db->updateBalance(pending.userId, pending.amount)) {
                m_db->recordTransaction("", pending.userId, pending.amount, "withdraw_reverted");
            } else {
                qWarning() << "checkPendingWithdrawConfirmations: failed to refund user" << pending.userId << "for slate" << pending.slateId;
            }

            m_pendingWithdraws.remove(pending.slateId);
            QString name = m_db->usernameByUserId(pending.userId);
            if (name.isEmpty()) {
                name = pending.userId;
            }
            QString notice = QString("Hi %1,\nyour withdrawal transaction (%2 GRIN) %3")
                                 .arg(name)
                                 .arg(formatGrin(pending.amount))
                                 .arg(cancellationReason);
            sendUserDirectMessage(pending.userId, notice, true, nullptr);

            if (!m_db->markPendingWithdrawCompleted(pending.slateId)) {
                qWarning() << "Failed to mark pending withdraw completed" << pending.slateId;
            }
        }
    }

    QList<PendingWithdrawConfirmationRecord> pendingList = m_db->pendingWithdrawConfirmations();
    if (pendingList.isEmpty()) {
        return;
    }

    for (const PendingWithdrawConfirmationRecord &pending : pendingList) {
        qDebug() << "pendingList:  " << pending.firstName << "   " << pending.slateId;

        if (pending.slateId.isEmpty()) {
            continue;
        }

        if (!txBySlate.contains(pending.slateId)) {
            qDebug() << "checkPendingWithdrawConfirmations: slate" << pending.slateId << "not found; assuming cleanup removed tx";
            if (!m_db->markPendingWithdrawConfirmationCompleted(pending.slateId)) {
                qWarning() << "Failed to mark missing pending withdraw confirmation" << pending.slateId;
            }
            QString notice = QString("Hi %1,\ndeine Auszahlungs-Transaktion wurde vom Wallet zurückgezogen. Bitte erneut anstoßen.")
                                 .arg(pending.firstName.isEmpty() ? pending.userId : pending.firstName);
            sendUserDirectMessage(pending.userId, notice, true, nullptr);
            continue;
        }

        const TxLogEntry entry = txBySlate.value(pending.slateId);
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
        sendUserDirectMessage(pending.userId, msg, true, nullptr);

        if (!m_db->markPendingWithdrawConfirmationCompleted(pending.slateId)) {
            qWarning() << "Failed to mark pending withdraw confirmation" << pending.slateId;
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

void TippingWorker::sendUserDirectMessage(const QString &userId, QString content, bool plain, const TelegramBotMessage *context, bool sendToUserChat)
{
    Q_UNUSED(plain);
    if (userId.isEmpty()) {
        return;
    }

    qlonglong chatId = -1;
    if (sendToUserChat) {
        if (m_db && m_db->userHasPrivateChat(userId)) {
            bool ok = false;
            qlonglong candidate = userId.toLongLong(&ok);
            if (ok && candidate > 0) {
                chatId = candidate;
            }
        }
        if (chatId <= 0 && context) {
            if (context->chat.id > 0) {
                chatId = context->chat.id;
            } else if (context->from.id > 0) {
                chatId = context->from.id;
            }
        }
    } else if (context) {
        chatId = context->chat.id;
    }

    if (chatId <= 0) {
        qWarning() << "Missing private chat id for user" << userId << "while sending direct message";
        if (sendToUserChat && context && context->chat.id < 0) {
            sendUserMessage(*context,
                            "Please send me a private message and press start once so i can reply to you directly in the future and avoid cluttering the group chat.",
                            false,
                            false);
        }
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

    qlonglong targetChatId = message.chat.id;
    if (sendToUserChat) {
        QString senderId = QString::number(message.from.id);
        qlonglong senderChatId = message.from.id;
        const bool hasPrivateChat = m_db && !senderId.isEmpty() && m_db->userHasPrivateChat(senderId);
        if (!hasPrivateChat) {
            senderChatId = -1;
        }
        if (senderChatId > 0) {
            targetChatId = senderChatId;
        } else if (message.chat.id > 0) {
            targetChatId = message.chat.id;
        } else if (message.from.id > 0) {
            targetChatId = message.from.id;
        } else {
            qWarning() << "Cannot determine private chat id for user" << senderId;
            return;
        }
    }
    QVariant chatId = QVariant::fromValue(targetChatId);
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

    qlonglong targetChatId = message.chat.id;
    if (sendToUserChat) {
        QString senderId = QString::number(message.from.id);
        qlonglong senderChatId = message.from.id;
        const bool hasPrivateChat = m_db && !senderId.isEmpty() && m_db->userHasPrivateChat(senderId);
        if (!hasPrivateChat) {
            senderChatId = -1;
        }
        if (senderChatId > 0) {
            targetChatId = senderChatId;
        } else if (message.chat.id > 0) {
            targetChatId = message.chat.id;
        } else if (message.from.id > 0) {
            targetChatId = message.from.id;
        } else {
            qWarning() << "Cannot determine private chat id for user" << senderId;
            return;
        }
    }
    QVariant chatId = QVariant::fromValue(targetChatId);
    m_bot->sendMessage(chatId,
                       msg,
                       0,
                       TelegramBot::Markdown | TelegramBot::DisableWebPagePreview,
                       TelegramKeyboardRequest(),
                       nullptr);
}

/**
 * @brief TippingWorker::resolveRecipientId
 * @param target
 * @param message
 * @return
 */
QString TippingWorker::resolveRecipientId(const QString &target,
                                          const TelegramBotMessage &message) const
{
    auto isValidUserId = [](qlonglong id) -> bool {
        return id > 0;
    };

    QString normalized = target.trimmed();

    qDebug() << "[resolveRecipientId] raw target =" << target;
    qDebug() << "[resolveRecipientId] normalized =" << normalized;
    qDebug() << "[resolveRecipientId] message.text =" << message.text;
    qDebug() << "[resolveRecipientId] from.id =" << message.from.id
             << "chat.id =" << message.chat.id
             << "from.username =" << message.from.username
             << "from.firstName =" << message.from.firstName;
    qDebug() << "[resolveRecipientId] entities.count =" << message.entities.size();

    // Strip leading "@"
    if (normalized.startsWith("@")) {
        normalized = normalized.mid(1).trimmed();
        qDebug() << "[resolveRecipientId] stripped leading @ =>" << normalized;
        QString uid = m_db->userIdByUsername(normalized);
        return uid;
    }

    // Helper: iterate entities safely (offset/length in UTF-16 units)
    auto entityTextAt = [&](const TelegramBotMessageEntity &entity) -> QString {
        const int o = qMax(0, entity.offset);
        const int l = qMax(0, entity.length);
        if (o >= message.text.size() || l <= 0) return {};
        return message.text.mid(o, qMin(l, message.text.size() - o));
    };

    // 0) If user replied to someone: prefer reply_to_message.from.id
    // (Typical UX: /tip 1 as reply)
    if (message.replyToMessage.messageId != 0) {
        const qlonglong replyUid = static_cast<qlonglong>(message.replyToMessage.from.id);
        if (isValidUserId(replyUid)) {
            qDebug() << "[resolveRecipientId] using reply_to_message.from.id =" << replyUid;
            return QString::number(replyUid);
        }
        qDebug() << "[resolveRecipientId] reply_to_message.from.id invalid =" << replyUid;
    }

    // 1) No explicit target:
    //    Prefer first valid text_mention entity (contains user object with id).
    if (normalized.isEmpty()) {
        qDebug() << "[resolveRecipientId] no explicit target: scanning for text_mention...";
        for (int i = 0; i < message.entities.size(); ++i) {
            const auto &entity = message.entities.at(i);
            const qlonglong uid = static_cast<qlonglong>(entity.user.id);

            qDebug() << "[resolveRecipientId] entity" << i
                     << "type=" << entity.type
                     << "offset=" << entity.offset
                     << "length=" << entity.length
                     << "text=" << entityTextAt(entity)
                     << "user.id=" << uid;

            if (entity.type == "text_mention" && isValidUserId(uid)) {
                qDebug() << "[resolveRecipientId] FOUND text_mention user.id =" << uid;
                return QString::number(uid);
            }
        }

        // If no text_mention, we cannot resolve a plain @mention to an ID without a cache.
        qDebug() << "[resolveRecipientId] no explicit target and no valid text_mention -> return empty";
        return {};
    }

    // 2) Numeric target:
    //    Accept ONLY positive user ids.
    {
        bool ok = false;
        const qlonglong id = normalized.toLongLong(&ok);

        qDebug() << "[resolveRecipientId] numeric parse ok=" << ok << "value=" << id;

        if (ok) {
            if (isValidUserId(id)) {
                qDebug() << "[resolveRecipientId] numeric target accepted as user id:" << id;
                return QString::number(id);
            }
            qDebug() << "[resolveRecipientId] numeric target REJECTED (non-user id, likely chat/group/channel):" << id;
            return {};
        }
    }

    // 3) Entity scan:
    //    - text_mention => ALWAYS return uid if valid (do NOT depend on text match)
    //    - mention => can only match username text (no uid provided by Telegram)
    qDebug() << "[resolveRecipientId] scanning entities for text_mention / mention...";
    for (int i = 0; i < message.entities.size(); ++i) {
        const auto &entity = message.entities.at(i);

        const QString entityText = entityTextAt(entity);
        QString stripped = entityText.trimmed();
        if (stripped.startsWith("@"))
            stripped = stripped.mid(1).trimmed();

        const qlonglong uid = static_cast<qlonglong>(entity.user.id);

        qDebug() << "[resolveRecipientId] entity" << i
                 << "type=" << entity.type
                 << "offset=" << entity.offset
                 << "length=" << entity.length
                 << "entityText=" << entityText
                 << "stripped=" << stripped
                 << "user.id=" << uid;

        if (entity.type == "text_mention") {
            if (isValidUserId(uid)) {
                qDebug() << "[resolveRecipientId] FOUND text_mention user.id =" << uid
                         << "(ignoring text match; entityText=" << entityText << ")";
                return QString::number(uid);
            }
            qDebug() << "[resolveRecipientId] REJECT text_mention user.id (invalid/non-user):" << uid;
            continue;
        }

        if (entity.type == "mention") {
            // mention gives only username text, no uid. We can confirm match,
            // but we cannot convert to id without a username->id cache.
            if (!stripped.isEmpty() && stripped.compare(normalized, Qt::CaseInsensitive) == 0) {
                qDebug() << "[resolveRecipientId] MATCH mention username (no user.id provided by Telegram):"
                         << "normalized=" << normalized
                         << "mentionText=" << stripped
                         << "-> cannot resolve without cache";
                return {};
            }
        }
    }

    // 4) Optional: fallback to cache lookup (username/firstName -> id)
    // If you have something like:
    //   qlonglong uid = this->userCacheResolve(normalized);
    // then enable it here:
    //
    // const qlonglong cached = userCacheResolve(normalized);
    // if (isValidUserId(cached)) return QString::number(cached);

    qDebug() << "[resolveRecipientId] no resolvable recipient id found -> return empty";
    return {};
}
