#include "tippingworker.h"

#include <QtGlobal>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QVariant>
#include <QJsonArray>
#include <QStringList>
#include <QList>
#include "txlogentry.h"

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
    QString text = message.text.trimmed();

    if (!message.document.fileId.isEmpty() && !message.document.fileName.isEmpty()) {
        return handleSlatepackDocument(message);
    }

    if (text.contains("BEGINSLATEPACK") && text.contains("ENDSLATEPACK")) {
        return handleSlatepackText(message, text);
    }

    const QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return false;

    const QString cmd = parts[0];
    const QString senderId = QString::number(message.from.id);
    const QString sender = userLabel(message);

    if (cmd == "/adminamounts") {
        if (!isAdmin(message.from.id)) {
            sendUserMessage(message, "You are not authorized to run this command.", false);
            return true;
        }
        sendUserMessage(message, handleAdminAmountsCommand(), false);
        return true;
    }

    if (cmd == "/deposit") {
        if (parts.size() != 2) {
            sendUserMessage(message, "Usage: /deposit <amount in GRIN>", false);
            return true;
        }
        bool ok;
        int amount = parts[1].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserMessage(message, "Please provide a positive amount.", false);
            return true;
        }
        sendUserMessage(message, handleDepositCommand(senderId, amount, message), false);
        return true;
    }

    if (cmd == "/withdraw") {
        if (parts.size() != 2) {
            sendUserMessage(message, "Usage: /withdraw <amount in GRIN>", false);
            return true;
        }
        bool ok;
        int amount = parts[1].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserMessage(message, "Please provide a positive amount.", false);
            return true;
        }
        sendUserMessage(message, handleWithdrawCommand(senderId, amount, message), false);
        return true;
    }

    if (cmd == "/tip") {
        if (parts.size() != 3) {
            sendUserMessage(message, "Usage: /tip @user <amount>", false);
            return true;
        }
        QString toUser = parts[1];
        if (toUser.startsWith("@")) toUser = toUser.mid(1);
        bool ok;
        int amount = parts[2].toInt(&ok);
        if (!ok || amount <= 0) {
            sendUserMessage(message, "Please provide a positive amount.", false);
            return true;
        }
        sendUserMessage(message, handleTip(senderId, sender, toUser, amount), false);
        return true;
    }

    if (cmd == "/balance") {
        int bal = m_db->getBalance(senderId);
        sendUserMessage(message, QString("Your current balance: %1 GRIN").arg(bal), false);
        return true;
    }

    if (cmd == "/opentxs") {
        sendUserMessage(message, handleOpenTransactionsCommand(sender), false);
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
        Result<QString> result = handleSlateS2State(slate, message);
        QString info;
        if (!result.unwrapOrLog(info)) {
            sendUserMessage(message, QString("Error: %1").arg(result.errorMessage()), false);
            return true;
        }
        sendUserMessage(message, QString("Withdraw completed: %1").arg(info), false);
        m_pendingWithdraws.remove(slateId);
        if (!slateId.isEmpty() && !m_db->removePendingWithdraw(slateId)) {
            qWarning() << "Failed to remove pending withdraw" << slateId;
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
        Result<QString> result = handleSlateS2State(slate, message);
        QString info;
        if (!result.unwrapOrLog(info)) {
            sendUserMessage(message, QString("Error: %1").arg(result.errorMessage()), false);
            return true;
        }
        sendUserMessage(message, QString("Withdraw completed: %1").arg(info), false);
        m_pendingWithdraws.remove(slateId);
        if (!slateId.isEmpty() && !m_db->removePendingWithdraw(slateId)) {
            qWarning() << "Failed to remove pending withdraw" << slateId;
        }
        return true;
    }

    return false;
}

QString TippingWorker::handleDepositCommand(const QString &senderId, int amount, TelegramBotMessage message)
{
    Q_UNUSED(senderId);
    qlonglong nanogrin = qlonglong(amount) * 1000000000LL;
    QString slateId;
    Result<QString> res = createInvoiceSlatepack(nanogrin, slateId);
    QString slatepack;
    if (!res.unwrapOrLog(slatepack)) {
        return QString("Error creating deposit slatepack: %1").arg(res.errorMessage());
    }

    sendSlatepackMessage(message, slatepack, "I1");
    if (!slateId.isEmpty()) {
        PendingDepositRecord pending;
        pending.slateId = slateId;
        pending.userId = senderId;
        pending.chatId = message.chat.id;
        pending.firstName = message.from.firstName;
        pending.amount = amount;
        if (!m_db->insertPendingDeposit(pending)) {
            qWarning() << "Failed to store pending deposit for slate" << slateId;
        }
        qDebug() << "Deposit command stored pending slate" << slateId << "for" << pending.userId << "amount" << pending.amount;
    }
    return QString("The following file contains your Slatepack (I1).\nSend the completed Slatepack file (I2) back or paste it as text.\nInvoice for %1 GRIN has been created. Follow the instructions.").arg(amount);
}

QString TippingWorker::handleWithdrawCommand(const QString &senderId, int amount, TelegramBotMessage message)
{
    if (m_db->getBalance(senderId) < amount) {
        return "Insufficient balance.";
    }

    qlonglong nanogrin = qlonglong(amount) * 1000000000LL;
    Result<QString> res = createSendSlatepack(nanogrin, senderId);
    QString slatepack;
    if (!res.unwrapOrLog(slatepack)) {
        return QString("Error creating withdraw slatepack: %1").arg(res.errorMessage());
    }

    sendSlatepackMessage(message, slatepack, "S1");
    return QString("The following file contains your Slatepack (S1).\nSend the completed Slatepack file (S2) back or paste it as text.\nA send transaction for %1 GRIN has been prepared. Follow the instructions.").arg(amount);
}

QString TippingWorker::handleTip(const QString &fromId, const QString &fromLabel, const QString &toUser, int amount)
{
    if (!fromLabel.isEmpty() && fromLabel == toUser) {
        return "You cannot tip yourself.";
    }
    if (toUser == fromId) {
        return "You cannot tip yourself.";
    }

    if (m_db->getBalance(fromId) < amount) {
        return "Insufficient balance.";
    }

    m_db->updateBalance(fromId, -amount);
    m_db->updateBalance(toUser, amount);
    m_db->recordTransaction(fromId, toUser, amount, "tip");
    return QString("%1 GRIN sent to @%2.").arg(amount).arg(toUser);
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
            qlonglong grinAmount = static_cast<qlonglong>(amount / 1000000000LL);
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
            lines << QString("Pending deposit %1 GRIN for %2 (slate %3)").arg(pending.amount).arg(name).arg(pending.slateId);
        }
    }

    if (lines.isEmpty()) {
        return "No open transactions found.";
    }

    QString header = QString("Open transactions (%1):").arg(lines.size());
    lines.prepend(header);
    return lines.join("\n");
}

void TippingWorker::sendSlatepackMessage(TelegramBotMessage message, const QString &slatepack, const QString &stateLabel)
{
    QString fileName = QString("%1.%2.slatepack").arg(userLabel(message)).arg(stateLabel);
    m_bot->sendDocument(fileName,
                        message.chat.id,
                        QVariant(slatepack.toUtf8() + "\n"),
                        "",
                        0,
                        TelegramBot::NoFlag,
                        TelegramKeyboardRequest(),
                        nullptr);
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

    QString slateId = slate.id();
    int amountGrin = static_cast<int>(nanogrin / 1000000000LL);
    if (!slateId.isEmpty()) {
        PendingWithdrawRecord pending{slateId, senderId, amountGrin, QDateTime::currentSecsSinceEpoch()};
        m_pendingWithdraws.insert(slateId, pending);
        if (!m_db->insertPendingWithdraw(pending)) {
            qWarning() << "createSendSlatepack: failed to persist pending withdraw" << slateId;
        }
        qDebug() << "createSendSlatepack: stored pending withdraw" << slateId << "amount" << amountGrin;
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
    int computedAmount = static_cast<int>(nanogrin / 1000000000LL);

    PendingDepositRecord pending;
    bool hasPending = false;
    if (!slateId.isEmpty()) {
        hasPending = m_db->pendingDeposit(slateId, pending);
    }

    int displayAmount = hasPending ? pending.amount : computedAmount;

    if (displayAmount <= 0) {
        return Error(ErrorType::Unknown, "Deposit amount missing.");
    }

    return QString("%1 GRIN deposit received. Waiting for confirmations before crediting.").arg(displayAmount);
}

Result<QString> TippingWorker::handleSlateS2State(Slate slate, TelegramBotMessage message)
{
    if (!ensureTippingAccount()) {
        return Error(ErrorType::Unknown, "Tipping account not available.");
    }

    Slate finalized;
    {
        qDebug() << "handleSlateS2State: finalizing withdraw slate for" << message.from.firstName;
        Result<Slate> res = m_walletOwnerApi->finalizeTx(slate);
        if (!res.unwrapOrLog(finalized)) {
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
    int grinAmount = static_cast<int>(nanogrin / 1000000000LL);
    QString user = userLabel(message);
    if (!m_db->updateBalance(user, -grinAmount)) {
        return Error(ErrorType::Unknown, "Error deducting from account.");
    }
    m_db->recordTransaction(user, "", grinAmount, "withdraw");

    return QString("%1 GRIN withdrawn.").arg(grinAmount);
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

        bool credited = false;
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

            int creditedAmount = pending.amount;
            quint64 amountNano = entry.amountCredited() > 0 ? entry.amountCredited() : entry.amountDebited();
            if (amountNano > 0) {
                creditedAmount = static_cast<int>(amountNano / 1000000000LL);
            }
            if (creditedAmount <= 0) {
                creditedAmount = pending.amount;
            }

            qDebug() << "checkPendingDeposits: crediting" << creditedAmount << "GRIN to" << pending.userId << "for tx" << entry.id();
            m_db->updateBalance(pending.userId, creditedAmount);
            m_db->recordTransaction("", pending.userId, creditedAmount, "deposit");

            QString reply = QString("%1 GRIN deposit confirmed and credited to your balance.").arg(creditedAmount);
            QString msg;
            if (!pending.firstName.isEmpty()) {
                msg = QString("Hi %1,\n%2").arg(pending.firstName).arg(reply);
            } else {
                msg = reply;
            }
            m_bot->sendMessage(pending.chatId,
                               msg,
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            if (!m_db->removePendingDeposit(slateId)) {
                qWarning() << "Failed to remove pending deposit" << slateId;
            } else {
                qDebug() << "checkPendingDeposits: removed pending deposit" << slateId;
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

void TippingWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi " + message.from.firstName + ",\n" + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}
