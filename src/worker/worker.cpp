#include "worker.h"

/**
 * @brief Worker::Worker
 */
Worker::Worker() :
    m_dbManager(nullptr),
    m_bot(nullptr),
    m_nodeOwnerApi(nullptr),
    m_nodeForeignApi(nullptr),
    m_walletOwnerApi(nullptr),
    m_walletForeignApi(nullptr),
    m_settings(nullptr)
{
    m_settings = new QSettings(QCoreApplication::applicationDirPath() + "/etc/settings.ini", QSettings::IniFormat);
}

/**
 * @brief Worker::initBot
 */
bool Worker::init()
{
    //Todo checking success instances
    bool success = true;

    // Node Owner Api Instance
    m_nodeOwnerApi = new NodeOwnerApi(m_settings->value("node/ownerUrl").toString(),
                                      m_settings->value("node/ownerApiKey").toString());

    //need to check connection/online
    m_nodeOwnerApi->getStatus();

    //Node Foreign Api Instance
    m_nodeForeignApi = new NodeForeignApi(m_settings->value("node/foreignUrl").toString(),
                                          m_settings->value("node/foreignApiKey").toString());

    //need to check connection/online
    m_nodeForeignApi->getVersion();

    //Wallet Owner Api Instance
    m_walletOwnerApi = new WalletOwnerApi(m_settings->value("wallet/ownerUrl").toString(),
                                          m_settings->value("wallet/user").toString(),
                                          m_settings->value("wallet/apiSecret").toString());

    m_walletOwnerApi->initSecureApi();
    m_walletOwnerApi->openWallet("", m_settings->value("wallet/password").toString());

    //Wallet Foreign Api Instance
    m_walletForeignApi = new WalletForeignApi(m_settings->value("wallet/foreignUrl").toString());    

    //DB Instance
    m_dbManager = new DatabaseManager();
    if (m_dbManager->connectToDatabase(QCoreApplication::applicationDirPath() + "/etc/database/database.db")) {
        // Database connection
        qDebug()<<"db connection success!";
    } else {
        qDebug()<<"Error: no db connection!";
        success = false;
    }

    //Bot - Instance
    m_bot = new TelegramBot(m_settings->value("bot/token").toString());
    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));
    m_bot->startMessagePulling();


    qDebug()<<"retrieveTxs: ";
    qDebug()<<m_walletOwnerApi->retrieveTxs();


    qDebug()<<"Cancel:";
    qDebug()<<m_walletOwnerApi->cancelTx("",17);
    qDebug()<<m_walletOwnerApi->cancelTx("",19);

    return success;
}

/**
 * @brief Worker::onMessage
 * @param update
 */
void Worker::onMessage(TelegramBotUpdate update)
{
    // only handle Messages
    if (update->type != TelegramBotMessageType::Message) {
        return;
    }

    // simplify message access
    TelegramBotMessage &message = *update->message;
    qlonglong id = message.chat.id;

    /// following commands exists
    /*
    start - introduction
    address - get slatepack address
    donate - dm slatepack address
    faucet - use faucet
    adminenabledisabledeposits - enable/disable deposits
    adminenabledisablewithdrawals - enable/disable withdrawals
    adminupdateresponsemessage - update bot response messages templates
    adminrequirednumberofresponse - set the required number of responses to approve the withdrawal
    adminprofilrequirementswithdrawl - set the profile requirements to approve the withdrawal
    adminapprovedwithdrawalamount - set the approved withdrawal amount
    adminamount - get account amounts
    */

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command start
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/start")) {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName + ",\n"
                           + readFileToString(QCoreApplication::applicationDirPath() + "/etc/messages/start.txt"),
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command address
    // Getting the address
    // 1. User runs `/address` command.
    // 2. Bot replies with slatepack address.
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/address")) {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName + ",\nhere is the Slatepack address:\n\n" + m_walletOwnerApi->getSlatepackAddress(),
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command donate
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/donate")) {
        // enable
        if (m_settings->value("admin/enableDisableDeposits").toInt() == 1) {
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName
                               + ",\nnice that you want to make a donation."+ "\n"
                               + "### Deposit protocol"+ "\n"
                               + "1) User runs '/donate' to get manual"+ "\n"
                               + "2) Send a Slatepack to donate GRIN"+ "\n"
                               + "3) Bot send repsonse Slatepack"+ "\n"
                               + "4) Finalize"+ "\n"
                               + "\n",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);
        }
        // disable
        else {
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\ndonate function currently disable!",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);
        }

        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command Slatepack
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("BEGINSLATEPACK") && message.text.contains("ENDSLATEPACK")) {

        QJsonObject slateObj = m_walletOwnerApi->slateFromSlatepackMessage(message.text);
        if(slateObj.isEmpty())
        {
            qWarning()<<"slate error";
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\n"
                               + "error to decrypt Slatepack!",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);
            return;
        }

        Slate slate = Slate::fromJson(slateObj);
        qDebug()<<"Amt: "<<slate.amt;
        qDebug()<<"Fee: "<<slate.fee;
        qDebug()<<"Id: "<< slate.id;
        qDebug()<<"Sta: "<<slate.sta;
        qDebug()<<"Ver: "<<slate.ver;

        bool ok = false;
        qlonglong amount = slate.amt.toLongLong(&ok);

        if (ok) {
            qDebug() << "Amount (qlonglong):" << amount;
        } else {
            qWarning() << "convert failed!";
        }

        switch (slateStateFromString(slate.sta)) {
            case SlateState::S1:
                // S1 - Standard: Sender hat Slate mit Inputs, Change, Nonce, Excess erstellt
                qDebug() << "Slate state: S1 (Standard Sender Init)";

                m_bot->sendMessage(id,
                                   handleSlateS1State(slateObj, message),
                                   0,
                                   TelegramBot::NoFlag,
                                   TelegramKeyboardRequest(),
                                   nullptr);
                break;

            case SlateState::S2:
                // S2 - Standard: Empfänger hat Outputs, Nonce, PartialSig beigefügt
                qDebug() << "Slate state: S2 (Standard Recipient Response)";
                m_bot->sendMessage(id,
                                   "Hi " + message.from.firstName + ",\n"
                                   + "function currently not implemented!\nSlate state: S2 (Standard Recipient Response)",
                                   0,
                                   TelegramBot::NoFlag,
                                   TelegramKeyboardRequest(),
                                   nullptr);

                break;

            case SlateState::S3:
                // S3 - Standard: Slate vollständig, bereit zum Posten
                qDebug() << "Slate state: S3 (Standard Finalized)";
                m_bot->sendMessage(id,
                                   "Hi " + message.from.firstName + ",\n"
                                   + "function currently not implemented!\nSlate state: S3 (Standard Finalized)",
                                   0,
                                   TelegramBot::NoFlag,
                                   TelegramKeyboardRequest(),
                                   nullptr);
                break;

            case SlateState::I1:
                // I1 - Invoice: Payee (Zahlungsempfänger) beginnt Transaktion
                qDebug() << "Slate state: I1 (Invoice Payee Init)";

                // enable
                if (m_settings->value("admin/enableDisableWithdrawals").toInt() == 1) {

                    m_bot->sendMessage(id,
                                       handleSlateI1State(slateObj, message),
                                       0,
                                       TelegramBot::NoFlag,
                                       TelegramKeyboardRequest(),
                                       nullptr);
                }
                // disable
                else {
                    m_bot->sendMessage(id,
                                       "Hi " + message.from.firstName + ",\nfaucet function currently disable!",
                                       0,
                                       TelegramBot::NoFlag,
                                       TelegramKeyboardRequest(),
                                       nullptr);
                }

                break;

            case SlateState::I2:
                // I2 - Invoice: Payer hat Inputs, Change und Signature beigefügt
                qDebug() << "Slate state: I2 (Invoice Payer Response)";
                m_bot->sendMessage(id,
                                   "Hi " + message.from.firstName + ",\n"
                                   + "function currently not implemented!\nSlate state: I2 (Invoice Payer Response)",
                                   0,
                                   TelegramBot::NoFlag,
                                   TelegramKeyboardRequest(),
                                   nullptr);
                break;

            case SlateState::I3:
                // I3 - Invoice: Slate vollständig, bereit zum Posten
                qDebug() << "Slate state: I3 (Invoice Finalized)";
                m_bot->sendMessage(id,
                                   "Hi " + message.from.firstName + ",\n"
                                   + "function currently not implemented!\nSlate state: I3 (Invoice Finalized)",
                                   0,
                                   TelegramBot::NoFlag,
                                   TelegramKeyboardRequest(),
                                   nullptr);
                break;

            case SlateState::Unknown:
            default:
                qWarning() << "Unknown Slate-State!";
                m_bot->sendMessage(id,
                                   "Hi " + message.from.firstName + ",\n"
                                   + "function currently not implemented!\nUnknown Slate-State!",
                                   0,
                                   TelegramBot::NoFlag,
                                   TelegramKeyboardRequest(),
                                   nullptr);
                break;
        }
        return;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // command faucet
    // ------------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/faucet")) {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName
                           + ",\nsend a Slatepack to get GRIN."+"\n"
                           +"### Withdrawal protocol"+"\n"
                           +"1) User runs '/faucet' to get manual"+"\n"
                           +"2) Send a Slatepack to receive GRIN"+"\n"
                           +"3) Bot send repsonse Slatepack"+"\n"
                           +"4) Finalize"+"\n"
                           + "\n",
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);
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

            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\n" + txt,
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

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

            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\n" + txt,
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminupdateresponsemessage
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminupdateresponsemessage")) {
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\nfunction currently not implemented!",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminrequirednumberofresponse
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminrequirednumberofresponse")) {
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\nfunction currently not implemented!",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminprofilrequirementswithdrawl
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminprofilrequirementswithdrawl")) {
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\nfunction currently not implemented!",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminapprovedwithdrawalamount
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminapprovedwithdrawalamount")) {
            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\nfunction currently not implemented!",
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            return;
        }

        // --------------------------------------------------------------------------------------------------------------------------------------
        // command adminamount
        // --------------------------------------------------------------------------------------------------------------------------------------
        if (message.text.contains("/adminamount")) {

            SummaryInfo sumInfo = SummaryInfo::fromJson(m_walletOwnerApi->retrieveSummaryInfo(true,1));

            QString info;
            info.append("amountAwaitingConfirmation: "+QString::number(sumInfo.amountAwaitingConfirmation)+"\n");
            info.append("amountAwaitingFinalization: "+QString::number(sumInfo.amountAwaitingFinalization)+"\n");
            info.append("amountCurrentlySpendable: "+QString::number(sumInfo.amountCurrentlySpendable)+"\n");
            info.append("amountImmature: "+QString::number(sumInfo.amountImmature)+"\n");
            info.append("amountLocked: "+QString::number(sumInfo.amountLocked)+"\n");
            info.append("amountReverted: "+QString::number(sumInfo.amountReverted)+"\n");
            info.append("lastConfirmedHeight: "+QString::number(sumInfo.lastConfirmedHeight)+"\n");
            info.append("minimumConfirmations: "+QString::number(sumInfo.minimumConfirmations)+"\n");
            info.append("total: "+QString::number(sumInfo.total)+"\n");

            m_bot->sendMessage(id,
                               "Hi " + message.from.firstName + ",\n"+info,
                               0,
                               TelegramBot::NoFlag,
                               TelegramKeyboardRequest(),
                               nullptr);

            return;
        }
    }
}

/**
 * @brief readFileToString
 * @param filePath
 * @return
 */
QString Worker::readFileToString(const QString &filePath)
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
 * @brief Worker::isAdmin
 * @param id
 * @return
 */
bool Worker::isAdmin(qlonglong id)
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
 * @brief Worker::handleSlateS1State
 * @param slate
 * @return
 */
QString Worker::handleSlateS1State(QJsonObject slate, TelegramBotMessage message)
{
    Slate slateAtt = Slate::fromJson(slate);

    QJsonObject slate2 = m_walletForeignApi->receiveTx(slate,"","");
    qDebug()<<"";
    qDebug()<<"receiveTx";
    qDebug()<<slate2;
    qDebug()<<"";

    QString slatepack = m_walletOwnerApi->createSlatepackMessage(slate2,QJsonArray() , 0);
    qDebug()<<"";
    qDebug()<<"createSlatepackMessage";
    qDebug() << "slatepack: " << slatepack;
    qDebug()<<"";

    Donate d(nullptr);
    d.setUserId(QString::number(message.from.id));
    d.setUsername(message.from.firstName);
    d.setAmount(slateAtt.amt);
    d.setDate(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    m_dbManager->insertDonate(d);

    return slatepack;

}

/**
 * @brief Worker::handleSlateI1State
 * @param slate
 * @return
 */
QString Worker::handleSlateI1State(QJsonObject slate,TelegramBotMessage message)
{
    Slate slateAtt = Slate::fromJson(slate);
    QJsonObject txData;



    if(slateAtt.amt.toLongLong() > 2000000000)
    {
        return QString("Hi "+message.from.firstName+",\n the faucet currently only outputs 2 GRIN per day per user.");
    }

    QString amountToday = m_dbManager->getFaucetAmountForToday(QString::number(message.from.id));

    qDebug()<<"amountToday: "<<amountToday;
    if(amountToday.toLongLong() >= 2000000000)
    {
        return QString("Hi "+message.from.firstName+",\n the faucet currently only outputs 2 GRIN per day per user.");
    }

    txData["src_acct_name"] = QJsonValue::Null;
    txData["amount"] = slateAtt.amt;
    txData["minimum_confirmations"] = 10;
    txData["max_outputs"] = 500;
    txData["num_change_outputs"] = 1;
    txData["selection_strategy_is_use_all"] = false;
    txData["target_slate_version"] = QJsonValue::Null;
    txData["payment_proof_recipient_address"] = QJsonValue::Null;
    txData["send_args"] = QJsonValue::Null;


    QJsonObject slate2 = m_walletOwnerApi->processInvoiceTx(slate,txData);

    Slate slate2Att = Slate::fromJson(slate2);
    qDebug()<<"slate2Att";
    qDebug()<<"Amt: "<<slate2Att.amt;
    qDebug()<<"fee: "<<slate2Att.fee;
    qDebug()<<"id: "<<slate2Att.id;
    qDebug()<<"sta: "<<slate2Att.sta;
    qDebug()<<"ver: "<<slate2Att.ver;

    //do something with slate
    QString slatepack = m_walletOwnerApi->createSlatepackMessage(slate2,QJsonArray() , 0);
    qDebug()<<"";
    qDebug()<<"createSlatepackMessage";
    qDebug() << "slatepack: " << slatepack;
    qDebug()<<"";

    qDebug()<<"Tx Lock Outputs";
    qDebug()<<m_walletOwnerApi->txLockOutputs(slate);

    Faucet f;
    f.setUserId(QString::number(message.from.id));
    f.setUsername(message.from.firstName);
    f.setAmount(slateAtt.amt);
    f.setDate(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    m_dbManager->insertFaucet(f);

    return slatepack;
}
