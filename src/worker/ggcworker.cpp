#include "ggcworker.h"

/**
 * @brief GgcWorker::GgcWorker
 */
GgcWorker::GgcWorker(TelegramBot *bot, QSettings *settings) :
    m_dbManager(nullptr),
    m_bot(bot),
    m_nodeOwnerApi(nullptr),
    m_nodeForeignApi(nullptr),
    m_walletOwnerApi(nullptr),
    m_walletForeignApi(nullptr),
    m_settings(settings),
    m_faucetAmount(1000000000)
{
}

/**
 * @brief GgcWorker::initBot
 */
bool GgcWorker::init()
{
    // Todo checking success instances
    bool success = true;

    // Node Owner Api Instance
    m_nodeOwnerApi = new NodeOwnerApi(m_settings->value("node/ownerUrl").toString(),
                                      m_settings->value("node/ownerApiKey").toString());

    // need to check connection/online
    m_nodeOwnerApi->getStatus();

    // Node Foreign Api Instance
    m_nodeForeignApi = new NodeForeignApi(m_settings->value("node/foreignUrl").toString(),
                                          m_settings->value("node/foreignApiKey").toString());

    // need to check connection/online
    m_nodeForeignApi->getVersion();

    // Wallet Owner Api Instance
    m_walletOwnerApi = new WalletOwnerApi(m_settings->value("wallet/ownerUrl").toString(),
                                          m_settings->value("wallet/user").toString(),
                                          m_settings->value("wallet/apiSecret").toString());

    m_walletOwnerApi->initSecureApi();
    m_walletOwnerApi->openWallet("", m_settings->value("wallet/password").toString());

    //scan whole wallet
    // if (!scanWallet()) {
    //     success = false;
    // }

    // Wallet Foreign Api Instance
    m_walletForeignApi = new WalletForeignApi(m_settings->value("wallet/foreignUrl").toString());

    // DB Instance
    m_dbManager = new GgcDatabaseManager();


    QString dbPath;
    QString dataDir = qEnvironmentVariable("DATA_DIR");

    if (dataDir.isEmpty()) {
        dbPath = QCoreApplication::applicationDirPath() + "/etc/database/database.db";
    }
    else
    {
        dbPath = QDir(dataDir).filePath("etc/database/database.db");
    }


    qDebug() << "DB Pfad GGC:" << dbPath;
    if (m_dbManager->connectToDatabase(dbPath)) {
        // Database connection
        qDebug() << "db connection success!";
    } else {
        qDebug() << "Error: no db connection!";
        success = false;
    }

    // Set Slot to bot message
    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));

    // Helper transactions cleanup
    QTimer *cleanupTimer = new QTimer(this);

    // Connect timer's timeout signal to your slot/function
    connect(cleanupTimer, &QTimer::timeout, this, &GgcWorker::cleanupRetrieveTxs);

    // Set interval to 5 minutes (300,000 milliseconds)
    cleanupTimer->start(5 * 60 * 1000);

    // Optional: call it once immediately at startup
    cleanupRetrieveTxs();

    return success;
}

/**
 * @brief GgcWorker::onMessage
 * @param update
 */
void GgcWorker::onMessage(TelegramBotUpdate update)
{
    // only handle Messages
    if (update->type != TelegramBotMessageType::Message) {
        return;
    }

    // simplify message access
    TelegramBotMessage &message = *update->message;
    qlonglong id = message.chat.id;

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command start
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/start")) {

        QString path;
        QString dataDir = qEnvironmentVariable("DATA_DIR");

        if (dataDir.isEmpty()) {
            path = QCoreApplication::applicationDirPath() + "/etc/messages/start.txt";
        }
        else
        {
            path = QDir(dataDir).filePath("etc/messages/start.txt");
        }

        sendUserMarkdownMessage(message, readFileToString(path), false);
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command address
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/address")) {
        QString str;
        {
            Result<QString> res = m_walletOwnerApi->getSlatepackAddress(0);
            if (!res.unwrapOrLog(str)) {
                str = QString("Error message: %1").arg(res.errorMessage());
            } else {
                str = QString("Hi %2,\nhere is my slatepack address:\n`%1`").arg(str).arg(message.chat.firstName);
            }
        }
        m_bot->sendMessage(message.chat.id, str, 0, TelegramBot::Markdown);
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command donatepack
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.startsWith("/donatepack")) {
        QStringList parts = message.text.split(" ", Qt::SkipEmptyParts);

        //Partsize size must 2
        if (parts.size() == 2) {
            bool ok = false;
            qlonglong amount = parts[1].toInt(&ok);

            if (ok && amount > 0) {
                // Valid /donatepack command with amount
                QString response;

                amount = amount*1000000000; //nano grin

                Result<Slate> resIssueInvoiceTx =  m_walletOwnerApi->issueInvoiceTx(QString::number(amount),"","");
                Slate slate;
                if (!resIssueInvoiceTx.unwrapOrLog(slate)) {
                    response = resIssueInvoiceTx.errorMessage();
                }
                else
                {
                    Result<QString> resCreateSlatepackMessage = m_walletOwnerApi->createSlatepackMessage(slate, QJsonArray(), 0);
                    if (!resCreateSlatepackMessage.unwrapOrLog(response)) {
                        response = resCreateSlatepackMessage.errorMessage();
                    }
                }
                sendUserMessage(message,response,false);

            }
            //invalid amount
            else {
                QString response = "Invalid amount. Please enter a positive number, e.g. /donatepack 10";
                sendUserMessage(message, response,false);
            }
        }
        //Missing or too many arguments
        else {
            QString response = "Usage: /donatepack <amount>\nExample: /donatepack 10";
            sendUserMessage(message, response,false);
        }

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command donate
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/donate")) {
        // enable
        if (m_settings->value("admin/enableDisableDeposits").toInt() == 1) {
            QString path;
            QString dataDir = qEnvironmentVariable("DATA_DIR");

            if (dataDir.isEmpty()) {
                path = QCoreApplication::applicationDirPath() + "/etc/messages/donate.txt";
            }
            else
            {
                path = QDir(dataDir).filePath("etc/messages/donate.txt");
            }

            sendUserMarkdownMessage(message, readFileToString(path), false);
            return;
        }
        // disable
        else {
            sendUserMessage(message, "donate function currently disable!", false);
        }

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // S1 File
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.document.fileName.endsWith("S1.slatepack")) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // TelegramBotFile - This object represents a file ready to be downloaded.
        // The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
        // It is guaranteed that the link will be valid for at least 1 hour. When the link expires,
        // a new one can be requested by calling getFile. Maximum file size to download is 20 MB
        // --------------------------------------------------------------------------------------------------------------------------------------
        TelegramBotFile file = m_bot->getFile(message.document.fileId);
        QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
        QString slatepack = downloadFileToQString(QUrl(link));

        if (slatepack.isEmpty()) {
            sendUserMessage(message, QString("Slatepack could not be extracted from the file: " + file.filePath), false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get Slate from Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get S2 Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString msg;
        {
            Result<QString> res = handleSlateS1State(slate, message);
            if (!res.unwrapOrLog(msg)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // send file S2
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString path = file.filePath; // e.g., "documents/1234-5678.S1.slatepack"
        QString baseName = QFileInfo(path).baseName(); // "1234-5678.S1"
        QString filename = baseName.section('.', 0, 0); // "1234-5678"

        if (filename.isEmpty()) {
            sendUserMessage(message, QString("filename could not be extracted from the file: " + file.filePath), false);
            return;
        }

        sendUserMessage(message, "the following message contains your Slatepack file!", false);

        m_bot->sendDocument(filename + ".S2.slatepack",
                            id,
                            QVariant(msg.toUtf8() + "\n"),
                            "",
                            0,
                            TelegramBot::NoFlag,
                            TelegramKeyboardRequest(),
                            nullptr);

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // S2 File
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.document.fileName.endsWith("S2.slatepack")) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // TelegramBotFile - This object represents a file ready to be downloaded.
        // The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
        // It is guaranteed that the link will be valid for at least 1 hour. When the link expires,
        // a new one can be requested by calling getFile. Maximum file size to download is 20 MB
        // --------------------------------------------------------------------------------------------------------------------------------------
        TelegramBotFile file = m_bot->getFile(message.document.fileId);
        QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
        QString slatepack = downloadFileToQString(QUrl(link));

        if (slatepack.isEmpty()) {
            sendUserMessage(message, QString("Slatepack could not be extracted from the file: " + file.filePath), false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get Slate from Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get S3 Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString msg;
        {
            Result<QString> res = handleSlateS2State(slate, message);
            if (!res.unwrapOrLog(msg)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // send file S3
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString path = file.filePath; // e.g., "documents/1234-5678.S2.slatepack"
        QString baseName = QFileInfo(path).baseName(); // "1234-5678.S2"
        QString filename = baseName.section('.', 0, 0); // "1234-5678"

        if (filename.isEmpty()) {
            sendUserMessage(message, QString("filename could not be extracted from the file: " + file.filePath), false);
            return;
        }

        sendUserMessage(message, "transaction finalized and broadcast, the following message contains your S3 slatepack file!", false);

        m_bot->sendDocument(filename + ".S3.slatepack",
                            id,
                            QVariant(msg.toUtf8() + "\n"),
                            "",
                            0,
                            TelegramBot::NoFlag,
                            TelegramKeyboardRequest(),
                            nullptr);

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // S3 File
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.document.fileName.endsWith("S3.slatepack")) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // TelegramBotFile - This object represents a file ready to be downloaded.
        // The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
        // It is guaranteed that the link will be valid for at least 1 hour. When the link expires,
        // a new one can be requested by calling getFile. Maximum file size to download is 20 MB
        // --------------------------------------------------------------------------------------------------------------------------------------
        TelegramBotFile file = m_bot->getFile(message.document.fileId);
        QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
        QString slatepack = downloadFileToQString(QUrl(link));

        if (slatepack.isEmpty()) {
            sendUserMessage(message, QString("Slatepack could not be extracted from the file: " + file.filePath), false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get Slate from Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // handle S3
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString msg;
        {
            Result<QString> res = handleSlateS3State(slate, message);
            if (!res.unwrapOrLog(msg)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        sendUserMessage(message, msg, false);
        return;
    }


    // ------------------------------------------------------------------------------------------------------------------------------------------
    // I1 File
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.document.fileName.endsWith("I1.slatepack")) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // TelegramBotFile - This object represents a file ready to be downloaded.
        // The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
        // It is guaranteed that the link will be valid for at least 1 hour. When the link expires,
        // a new one can be requested by calling getFile. Maximum file size to download is 20 MB
        // --------------------------------------------------------------------------------------------------------------------------------------
        TelegramBotFile file = m_bot->getFile(message.document.fileId);
        QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
        QString slatepack = downloadFileToQString(QUrl(link));

        if (slatepack.isEmpty()) {
            sendUserMessage(message, QString("Slatepack could not be extracted from the file: " + file.filePath), false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get Slate from Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get S2 Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString msg;
        {
            Result<QString> res = handleSlateI1State(slate, message);

            if (!res.unwrapOrLog(msg)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // send file S2
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString path = file.filePath; // e.g., "documents/1234-5678.I1.slatepack"
        QString baseName = QFileInfo(path).baseName(); // "1234-5678.I1"
        QString filename = baseName.section('.', 0, 0); // "1234-5678"

        if (filename.isEmpty()) {
            sendUserMessage(message, QString("filename could not be extracted from the file: " + file.filePath), false);
            return;
        }

        sendUserMessage(message, "the following message contains your Slatepack file!", false);

        m_bot->sendDocument(filename + ".I2.slatepack",
                            id,
                            QVariant(msg.toUtf8() + "\n"),
                            "",
                            0,
                            TelegramBot::NoFlag,
                            TelegramKeyboardRequest(),
                            nullptr);
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // I2 File
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.document.fileName.endsWith("I2.slatepack")) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // TelegramBotFile - This object represents a file ready to be downloaded.
        // The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
        // It is guaranteed that the link will be valid for at least 1 hour. When the link expires,
        // a new one can be requested by calling getFile. Maximum file size to download is 20 MB
        // --------------------------------------------------------------------------------------------------------------------------------------
        TelegramBotFile file = m_bot->getFile(message.document.fileId);
        QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
        QString slatepack = downloadFileToQString(QUrl(link));

        if (slatepack.isEmpty()) {
            sendUserMessage(message, QString("Slatepack could not be extracted from the file: " + file.filePath), false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get Slate from Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get I3 Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString msg;
        {
            Result<QString> res = handleSlateI2State(slate, message);
            if (!res.unwrapOrLog(msg)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // send file I3
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString path = file.filePath; // e.g., "documents/1234-5678.I2.slatepack"
        QString baseName = QFileInfo(path).baseName(); // "1234-5678.I2"
        QString filename = baseName.section('.', 0, 0); // "1234-5678"

        if (filename.isEmpty()) {
            sendUserMessage(message, QString("filename could not be extracted from the file: " + file.filePath), false);
            return;
        }

        sendUserMessage(message, "transaction finalized and broadcast, the following message contains your I3 slatepack file!", false);

        m_bot->sendDocument(filename + ".I3.slatepack",
                            id,
                            QVariant(msg.toUtf8() + "\n"),
                            "",
                            0,
                            TelegramBot::NoFlag,
                            TelegramKeyboardRequest(),
                            nullptr);

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // I3 File
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.document.fileName.endsWith("I3.slatepack")) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // TelegramBotFile - This object represents a file ready to be downloaded.
        // The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
        // It is guaranteed that the link will be valid for at least 1 hour. When the link expires,
        // a new one can be requested by calling getFile. Maximum file size to download is 20 MB
        // --------------------------------------------------------------------------------------------------------------------------------------
        TelegramBotFile file = m_bot->getFile(message.document.fileId);
        QString link = "https://api.telegram.org/file/bot" + m_settings->value("bot/token").toString() + "/" + file.filePath;
        QString slatepack = downloadFileToQString(QUrl(link));

        if (slatepack.isEmpty()) {
            sendUserMessage(message, QString("Slatepack could not be extracted from the file: " + file.filePath), false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // get Slate from Slatepack
        // --------------------------------------------------------------------------------------------------------------------------------------
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(slatepack);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // handle I3
        // --------------------------------------------------------------------------------------------------------------------------------------
        QString msg;
        {
            Result<QString> res = handleSlateI3State(slate, message);
            if (!res.unwrapOrLog(msg)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        sendUserMessage(message, msg, false);
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command Slatepack
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("BEGINSLATEPACK") && message.text.contains("ENDSLATEPACK")) {
        Slate slate;
        {
            Result<Slate> res = m_walletOwnerApi->slateFromSlatepackMessage(message.text);
            if (!res.unwrapOrLog(slate)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
        }

        SlateState state = Slate::slateStateFromString(slate.sta());

        if (state == SlateState::S1) {
            // S1 - Standard: Sender created Slate with Inputs, Change, Nonce, Excess
            qDebug() << "Slate state: S1 (Standard Sender Init)";

            QString msg;
            {
                Result<QString> res = handleSlateS1State(slate, message);
                if (!res.unwrapOrLog(msg)) {
                    sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                    return;
                }
                sendUserMessage(message, "here is your slatepack", false);
                sendUserMessage(message, msg, true);
            }
        } else if (state == SlateState::S2) {
            // S2 - Standard: Receiver added Outputs, Nonce, PartialSig
            qDebug() << "Slate state: S2 (Standard Recipient Response)";

            QString msg;
            {
                Result<QString> res = handleSlateS2State(slate, message);
                if (!res.unwrapOrLog(msg)) {
                    sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                    return;
                }
                sendUserMessage(message, "transaction finalized and broadcast", false);
                sendUserMessage(message, msg, true);
            }
        } else if (state == SlateState::S3) {
            // S3 - Standard: Slate complete, ready to post
            qDebug() << "Slate state: S3 (Standard Finalized)";

            QString msg;
            {
                Result<QString> res = handleSlateS3State(slate, message);
                if (!res.unwrapOrLog(msg)) {
                    sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                    return;
                }
                sendUserMessage(message, msg, false);
            }
        } else if (state == SlateState::I1) {
            // I1 - Invoice: Payee initiates transaction
            qDebug() << "Slate state: I1 (Invoice Payee Init)";

            if (m_settings->value("admin/enableDisableWithdrawals").toInt() == 1) {
                QString msg;
                {
                    Result<QString> res = handleSlateI1State(slate, message);
                    if (!res.unwrapOrLog(msg)) {
                        sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                        return;
                    }
                    sendUserMessage(message, "here is your slatepack", false);
                    sendUserMessage(message, msg, true);
                }
            } else {
                sendUserMessage(message, "faucet function currently disabled!", false);
            }
        } else if (state == SlateState::I2) {
            // I2 - Invoice: Payer added Inputs, Change and Signature
            qDebug() << "Slate state: I2 (Invoice Payer Response)";

            QString msg;
            {
                Result<QString> res = handleSlateI2State(slate, message);
                if (!res.unwrapOrLog(msg)) {
                    sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                    return;
                }
                sendUserMessage(message, "transaction finalized and broadcast", false);
                sendUserMessage(message, msg, true);
            }
        } else if (state == SlateState::I3) {
            // I3 - Invoice: Slate complete, ready to post
            qDebug() << "Slate state: I3 (Invoice Finalized)";

            QString msg;
            {
                Result<QString> res = handleSlateI3State(slate, message);
                if (!res.unwrapOrLog(msg)) {
                    sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                    return;
                }
                sendUserMessage(message, msg, false);
            }
        } else {
            // Unknown or unsupported Slate state
            qWarning() << "Unknown Slate-State!";
            sendUserMessage(message, "function currently not implemented!\nUnknown Slate-State!", false);
        }

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command faucetpack
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/faucetpack")) {

        QString response;
        InitTxArgs args;
        args.setSrcAcctName(QJsonValue::Null);
        args.setAmount(m_faucetAmount);
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

        qDebug()<<debugJsonString(args);

        Result<Slate> resInitSendTx =  m_walletOwnerApi->initSendTx(args);
        Slate slate;
        if (!resInitSendTx.unwrapOrLog(slate)) {
            response = resInitSendTx.errorMessage();
        }

        else
        {
            qDebug()<<debugJsonString(slate);
            Result<QString> resCreateSlatepackMessage = m_walletOwnerApi->createSlatepackMessage(slate, QJsonArray(), 0);
            if (!resCreateSlatepackMessage.unwrapOrLog(response)) {
                response = resCreateSlatepackMessage.errorMessage();
            }
        }

        ///---------------------------------------------------------------------------------------------------------------------------
        /// Handling txLockOutputs
        ///---------------------------------------------------------------------------------------------------------------------------
        //TODO lock on round 2, befove finalize?
        bool lockOutputs = false;
        {
            Result<bool> res = m_walletOwnerApi->txLockOutputs(slate);
            if (!res.unwrapOrLog(lockOutputs)) {
                response = res.errorMessage();
            } else {
                qDebug() << "txLockOutputs: " << lockOutputs;
            }
        }

        sendUserMessage(message, response, false);
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command faucet
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/faucet")) {
        QString path;
        QString dataDir = qEnvironmentVariable("DATA_DIR");

        if (dataDir.isEmpty()) {
            path = QCoreApplication::applicationDirPath() + "/etc/messages/faucet.txt";
        }
        else
        {
            path = QDir(dataDir).filePath("etc/messages/faucet.txt");
        }

        sendUserMarkdownMessage(message, readFileToString(path), false);
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command scanrewindhash
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/scanrewindhash")) {
        RewindHash rewindHash;
        {
            Result<RewindHash> res = m_walletOwnerApi->getRewindHash();
            if (!res.unwrapOrLog(rewindHash)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
            } else {
                ViewWallet viewWallet;
                {
                    QString msg;
                    Result<ViewWallet> res = m_walletOwnerApi->scanRewindHash(rewindHash, 1);
                    if (!res.unwrapOrLog(viewWallet)) {
                        msg = QString("Error message: %1").arg(res.errorMessage());
                    } else {
                        msg = debugJsonString(viewWallet);
                        m_bot->sendDocument("ScanRewindHash.json",
                                            id,
                                            QVariant(msg.toUtf8()),
                                            "",
                                            0,
                                            TelegramBot::NoFlag,
                                            TelegramKeyboardRequest(),
                                            nullptr);
                    }
                }
            }
        }
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command rewindhash
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/rewindhash")) {
        RewindHash rewindHash;
        {
            Result<RewindHash> res = m_walletOwnerApi->getRewindHash();
            if (!res.unwrapOrLog(rewindHash)) {
                sendUserMessage(message, QString("Error message: %1").arg(res.errorMessage()), false);
                return;
            }
            else {
                QString str = QString("Hi %2,\nhere is the rewind hash :\n`%1`").arg(rewindHash.rewindHash()).arg(message.chat.firstName);
                m_bot->sendMessage(message.chat.id, str, 0, TelegramBot::Markdown);
            }
        }
    return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // admin functions
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (isAdmin(message.from.id)) {
        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminenabledisabledeposits
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminenabledisabledeposits")) {
            QString txt;
            int currentState = m_settings->value("admin/enableDisableDeposits").toInt();

            if (currentState == 1) {
                currentState = 0;
                txt = "disable deposits!";
            } else {
                currentState = 1;
                txt = "enable deposits!";
            }

            m_settings->setValue("admin/enableDisableDeposits", currentState);
            sendUserMessage(message, txt, false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminenabledisablewithdrawals
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminenabledisablewithdrawals")) {
            QString txt;
            int currentState = m_settings->value("admin/enableDisableWithdrawals").toInt();

            if (currentState == 1) {
                currentState = 0;
                txt = "disable withdrawals!";
            } else {
                currentState = 1;
                txt = "enable withdrawals!";
            }

            m_settings->setValue("admin/enableDisableWithdrawals", currentState);
            sendUserMessage(message, txt, false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminupdateresponsemessage
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminupdateresponsemessage")) {
            sendUserMessage(message, "function currently not implemented!", false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminrequirednumberofresponse
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminrequirednumberofresponse")) {
            sendUserMessage(message, "function currently not implemented!", false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminprofilrequirementswithdrawl
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminprofilrequirementswithdrawl")) {
            sendUserMessage(message, "function currently not implemented!", false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminapprovedwithdrawalamount
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminapprovedwithdrawalamount")) {
            sendUserMessage(message, "function currently not implemented!", false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminamount
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminamount")) {
            QString info;
            WalletInfo walletInfo;
            {
                Result<WalletInfo> res = m_walletOwnerApi->retrieveSummaryInfo(true, 1);
                if (!res.unwrapOrLog(walletInfo)) {
                    info = QString("Error message: %1").arg(res.errorMessage());
                } else {
                    info.append("amountAwaitingConfirmation: " + QString::number(walletInfo.amountAwaitingConfirmation()) + "\n");
                    info.append("amountAwaitingFinalization: " + QString::number(walletInfo.amountAwaitingFinalization()) + "\n");
                    info.append("amountCurrentlySpendable: " + QString::number(walletInfo.amountCurrentlySpendable()) + "\n");
                    info.append("amountImmature: " + QString::number(walletInfo.amountImmature()) + "\n");
                    info.append("amountLocked: " + QString::number(walletInfo.amountLocked()) + "\n");
                    info.append("amountReverted: " + QString::number(walletInfo.amountReverted()) + "\n");
                    info.append("lastConfirmedHeight: " + QString::number(walletInfo.lastConfirmedHeight()) + "\n");
                    info.append("minimumConfirmations: " + QString::number(walletInfo.minimumConfirmations()) + "\n");
                    info.append("total: " + QString::number(walletInfo.total()) + "\n");
                }
            }

            sendUserMessage(message, info, false);
            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminfaucet
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminfaucet")) {
            QList<Faucet> list = m_dbManager->getAllFaucetAmountForToday();

            QString result;
            result += "ID | UserID | Username | Amount | Date\n";
            result += "----------------------------------------\n";

            for (const Faucet &faucet : list) {
                result += QString("%1 | %2 | %3 | %4 | %5\n")
                          .arg(faucet.id())
                          .arg(faucet.userId())
                          .arg(faucet.username())
                          .arg(faucet.amount())
                          .arg(faucet.date());
            }

            sendUserMessage(message, result, true);

            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command admindonate
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/admindonate")) {
            QList<Donate> list = m_dbManager->getAllDonate();

            QString result;
            result += "ID | UserID | Username | Amount | Date\n";
            result += "----------------------------------------\n";

            for (const Donate &donate : list) {
                result += QString("%1 | %2 | %3 | %4 | %5\n")
                          .arg(donate.id())
                          .arg(donate.userId())
                          .arg(donate.username())
                          .arg(donate.amount())
                          .arg(donate.date());
            }
            sendUserMessage(message, result, true);
        }
    }
}

/**
 * @brief readFileToString
 * @param filePath
 * @return
 */
QString GgcWorker::readFileToString(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Konnte Datei nicht öffnen:" << file.errorString();
        return {};
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

/**
 * @brief GgcWorker::isAdmin
 * @param id
 * @return
 */
bool GgcWorker::isAdmin(qlonglong id)
{
    // admin rights
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

/**
 * @brief GgcWorker::handleSlateS1State
 * @param slate
 * @return
 */
Result<QString> GgcWorker::handleSlateS1State(Slate slate, TelegramBotMessage message)
{
    ///---------------------------------------------------------------------------------------------------------------------------
    /// Debugging
    ///---------------------------------------------------------------------------------------------------------------------------
    qDebug() << "donate " << message.from.firstName << " :" << slate.amt();

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling receiveTx
    ///---------------------------------------------------------------------------------------------------------------------------
    Slate slate2;
    {
        Result<Slate> res = m_walletForeignApi->receiveTx(slate, "", "");
        if (!res.unwrapOrLog(slate2)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }
    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling createSlatepackMessage
    ///---------------------------------------------------------------------------------------------------------------------------
    QString slatepack;
    {
        Result<QString> res = m_walletOwnerApi->createSlatepackMessage(slate2, QJsonArray(), 0);
        if (!res.unwrapOrLog(slatepack)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling insert donate in db
    ///---------------------------------------------------------------------------------------------------------------------------
    Donate donate;
    donate.setUserId(QString::number(message.from.id));
    donate.setUsername(message.from.firstName);
    donate.setAmount(slate.amt());
    donate.setDate(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    qDebug() << "insert donate: " << m_dbManager->insertDonate(donate);

    return slatepack;
}

/**
 * @brief GgcWorker::handleSlateI1State
 * @param slate
 * @return
 */
Result<QString> GgcWorker::handleSlateI1State(Slate slate, TelegramBotMessage message)
{
    ///---------------------------------------------------------------------------------------------------------------------------
    /// Debugging
    ///---------------------------------------------------------------------------------------------------------------------------
    qDebug() << "faucet " << message.from.firstName << " :" << slate.amt();

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Check functions
    ///---------------------------------------------------------------------------------------------------------------------------
    QString amountToday = m_dbManager->getFaucetAmountForToday(QString::number(message.from.id));

    if (amountToday.toLongLong() >= m_faucetAmount || slate.amt().toLongLong() > m_faucetAmount) {
        return Error(ErrorType::Unknown,
                     QString("Hi " + message.from.firstName + ",\n the faucet currently only outputs 2 GRIN per day per user."));
    }

    ///---------------------------------------------------------------------------------------------------------------------------
    /// sync wallet
    ///---------------------------------------------------------------------------------------------------------------------------
    WalletInfo walletInfo;
    {
        Result<WalletInfo> res = m_walletOwnerApi->retrieveSummaryInfo(true, 1);
        if (!res.unwrapOrLog(walletInfo)) {
            return QString("Error message: %1").arg(res.errorMessage());
        }
    }
    ///---------------------------------------------------------------------------------------------------------------------------
    /// create args
    ///---------------------------------------------------------------------------------------------------------------------------
    QJsonObject txData;
    txData["src_acct_name"] = QJsonValue::Null;
    txData["amount"] = slate.amt();
    txData["minimum_confirmations"] = 10;
    txData["selection_strategy_is_use_all"] = false;

    // default
    txData["amount_includes_fee"] = QJsonValue::Null;
    txData["max_outputs"] = 500;
    txData["num_change_outputs"] = 1;
    txData["target_slate_version"] = QJsonValue::Null;
    txData["ttl_blocks"] = QJsonValue::Null;
    txData["estimate_only"] = false;
    txData["payment_proof_recipient_address"] = QJsonValue::Null;
    txData["late_lock"] = false;
    txData["send_args"] = QJsonValue::Null;

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling processInvoiceTx
    ///---------------------------------------------------------------------------------------------------------------------------
    Slate slate2;
    {
        Result<Slate> res = m_walletOwnerApi->processInvoiceTx(slate, txData);
        if (!res.unwrapOrLog(slate2)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling txLockOutputs
    ///---------------------------------------------------------------------------------------------------------------------------
    bool lockOutputs = false;
    {
        Result<bool> res = m_walletOwnerApi->txLockOutputs(slate);
        if (!res.unwrapOrLog(lockOutputs)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        } else {
            qDebug() << "txLockOutputs: " << lockOutputs;
        }
    }

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling createSlatepackMessages
    ///---------------------------------------------------------------------------------------------------------------------------
    QString slatepack;
    {
        Result<QString> res = m_walletOwnerApi->createSlatepackMessage(slate2, QJsonArray(), 0);
        if (!res.unwrapOrLog(slatepack)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling insert faucet in db
    ///---------------------------------------------------------------------------------------------------------------------------
    Faucet f;
    f.setUserId(QString::number(message.from.id));
    f.setUsername(message.from.firstName);
    f.setAmount(slate.amt());
    f.setDate(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    qDebug() << "insert faucet: " << m_dbManager->insertFaucet(f);

    return slatepack;
}

/**
 * @brief GgcWorker::handleSlateS2State
 * @param slate
 * @return
 */
Result<QString> GgcWorker::handleSlateS2State(Slate slate, TelegramBotMessage message)
{
    qDebug() << "finalize standard slate (S2) from" << message.from.firstName << ":" << slate.amt();

    Slate slate3;
    {
        Result<Slate> res = m_walletOwnerApi->finalizeTx(slate);
        if (!res.unwrapOrLog(slate3)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    bool posted = false;
    {
        Result<bool> res = m_walletOwnerApi->postTx(slate3, false);
        if (!res.unwrapOrLog(posted)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
        qDebug() << "postTx result:" << posted;
    }

    QString slatepack;
    {
        Result<QString> res = m_walletOwnerApi->createSlatepackMessage(slate3, QJsonArray(), 0);
        if (!res.unwrapOrLog(slatepack)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    return slatepack;
}

/**
 * @brief GgcWorker::handleSlateS3State
 * @param slate
 * @return
 */
Result<QString> GgcWorker::handleSlateS3State(Slate slate, TelegramBotMessage message)
{
    Q_UNUSED(slate);
    qDebug() << "received S3 slate from" << message.from.firstName;
    return QString("Slate state S3 acknowledged. Nothing more to do.");
}

/**
 * @brief GgcWorker::handleSlateI2State
 * @param slate
 * @return
 */
Result<QString> GgcWorker::handleSlateI2State(Slate slate, TelegramBotMessage message)
{
    qDebug() << "finalize invoice slate (I2) from" << message.from.firstName << ":" << slate.amt();

    Slate slate3;
    {
        Result<Slate> res = m_walletOwnerApi->finalizeTx(slate);
        if (!res.unwrapOrLog(slate3)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    bool posted = false;
    {
        Result<bool> res = m_walletOwnerApi->postTx(slate3, false);
        if (!res.unwrapOrLog(posted)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
        qDebug() << "postTx result:" << posted;
    }

    QString slatepack;
    {
        Result<QString> res = m_walletOwnerApi->createSlatepackMessage(slate3, QJsonArray(), 0);
        if (!res.unwrapOrLog(slatepack)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    return slatepack;
}

/**
 * @brief GgcWorker::handleSlateI3State
 * @param slate
 * @return
 */
Result<QString> GgcWorker::handleSlateI3State(Slate slate, TelegramBotMessage message)
{
    Q_UNUSED(slate);
    qDebug() << "received I3 slate from" << message.from.firstName;
    return QString("Slate state I3 acknowledged. Nothing more to do.");
}

/**
 * @brief GgcWorker::outputRetrieveTxs
 */
void GgcWorker::cleanupRetrieveTxs()
{
    QList<TxLogEntry> txList;
    {
        Result<QList<TxLogEntry> > res = m_walletOwnerApi->retrieveTxs(true, 0, "");
        if (!res.unwrapOrLog(txList)) {
            qDebug() << QString("Error message: %1").arg(res.errorMessage());
            return;
        }
    }

    for (int i = 0; i < txList.length(); i++) {
        // broken transactions
        if (txList[i].confirmed() == false && (txList[i].txType() == "TxReceived" || txList[i].txType() == "TxSent")) {
            QDateTime now = QDateTime::currentDateTimeUtc();

            if (txList[i].creationTs().secsTo(now) > 36000) { // Older than 10 hours
                qDebug() << "Transaction is older than 10 hours:";
                qInfo().noquote() << debugJsonString(txList[i]);

                // Cancel the old transaction
                bool cancelTx = false;
                {
                    Result<bool> res = m_walletOwnerApi->cancelTx("", txList[i].id());
                    if (!res.unwrapOrLog(cancelTx)) {
                        qDebug() << QString("Error message: %1").arg(res.errorMessage());
                    } else {
                        qDebug() << "cancelTx = " << cancelTx;
                    }
                }
            }
        }
    }
}

/**
 * @brief GgcWorker::downloadFileToQString
 * @param url
 * @return
 */
QString GgcWorker::downloadFileToQString(const QUrl &url)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(url);

    QNetworkReply *reply = manager.get(request);

    // Event Loop, um synchron zu warten bis der Download fertig ist
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply->readAll();
        result = QString::fromUtf8(bytes);
    } else {
        qWarning() << "Download Fehler:" << reply->errorString();
    }

    reply->deleteLater();
    return result;
}

/**
 * @brief GgcWorker::sendUserMessage
 * @param message
 * @param content
 */
void GgcWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi "
                      + message.from.firstName
                      + ",\n"
                      + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}

void GgcWorker::sendUserMarkdownMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi "
                      + message.from.firstName
                      + ",\n"
                      + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::Markdown | TelegramBot::DisableWebPagePreview,
                       TelegramKeyboardRequest(),
                       nullptr);
}


/**
 * @brief GgcWorker::scanWallet
 * @return
 */
bool GgcWorker::scanWallet()
{
    bool scan = false;
    {
        Result<bool> res = m_walletOwnerApi->scan(1, true);
        if (!res.unwrapOrLog(scan)) {
            qDebug() << res.errorMessage();
            scan = false;
        } else {
            scan = true;
            qDebug() << "scan: " << scan;
        }
    }
    return scan;
}
