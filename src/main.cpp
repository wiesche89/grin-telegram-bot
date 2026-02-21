#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSslSocket>

#include "grinwalletmanager.h"
#include "ggcworker.h"
#include "tippingworker.h"
#include "gateioworker.h"
#include "messagehub.h"
#include "api/wallet/owner/walletownerapi.h"

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
    QApplication  a(argc, argv);

    // Manager
    GrinWalletManager manager;
    if (!manager.startWallet()) {
        return -10;
    }

    // Start worker, wait x msecs
    QTimer::singleShot(3000, [&]() {

        QString settingsPath;
        QString dataDir;

        dataDir = qEnvironmentVariable("DATA_DIR");

        // native
        if (dataDir.isEmpty()) {
            settingsPath = QDir(QCoreApplication::applicationDirPath()).filePath("etc/settings.ini");
        }
        // docker
        else
        {
            settingsPath = QDir(dataDir).filePath("etc/settings.ini");
        }

        // check existing Config
        if (!QFile::exists(settingsPath)) {
            qWarning() << "Settings file not found:" << settingsPath;
            QCoreApplication::quit();
        } else {
            qDebug() << "Settings file found at:" << settingsPath;
        }

        // instance config
        QSettings *settings = new QSettings(settingsPath, QSettings::IniFormat);


        /// following commands exists
        /*
start - introduction
address - get slatepack address
donate - donate instruction
donatepack - bot initiate donation
faucet - use faucet
faucetpack - bot initiate faucet
rewindhash - get rewindhash
scanrewindhash - scan current rewindhash
price - price USDT
orderbook - current orderbook
chart - chart 4h USDT
history - last 10 trades USDT
deposit - example /deposit 10
withdraw - example /withdraw 10
tip - tipping Grins to other user example /tip @user 10
tipping - info about tipping function
ledger - shows outgoing incoming tips
balance - shows your balance

        Admin commands
        adminenabledisabledeposits - enable/disable deposits
        adminenabledisablewithdrawals - enable/disable withdrawals
        adminupdateresponsemessage - update bot response messages templates
        adminrequirednumberofresponse - set the required number of responses to approve the withdrawal
        adminprofilrequirementswithdrawl - set the profile requirements to approve the withdrawal
        adminapprovedwithdrawalamount - set the approved withdrawal amount
        adminamount - get account amounts
        admincleanup - cleanup txs
        */

        // Bot - Instance
        TelegramBot *bot = new TelegramBot(settings->value("bot/token").toString());

        WalletOwnerApi *walletOwnerApi = new WalletOwnerApi(settings->value("wallet/ownerUrl").toString(),
                                                            settings->value("wallet/user").toString(),
                                                            settings->value("wallet/apiSecret").toString(),
                                                            bot);
        walletOwnerApi->initSecureApi();
        walletOwnerApi->openWallet("", settings->value("wallet/password").toString());

        GgcWorker *ggcWorker = new GgcWorker(bot,settings,walletOwnerApi);
        if (!ggcWorker->init()) {
            qDebug()<<"GGC Worker init failed!";
            QCoreApplication::quit();
        }

        TippingWorker *tippingWorker = new TippingWorker(bot,settings,walletOwnerApi);
        if (!tippingWorker->init()) {
            qDebug()<<"Tipping Worker init failed!";
            QCoreApplication::quit();
        }

        GateIoWorker *gateIoWorker = new GateIoWorker(bot,settings);
        if (!gateIoWorker->init()) {
            qDebug()<<"GateIo Worker init failed!";
            QCoreApplication::quit();
        }

        new MessageHub(bot, tippingWorker, ggcWorker, bot);

        auto startPolling = [&]() {
            bot->startMessagePulling();
        };

        QString settingsDir = QFileInfo(settingsPath).absolutePath();
        auto resolvePath = [&](const QString &value) {
            if (value.isEmpty()) {
                return QString();
            }
            QFileInfo info(value);
            if (info.isAbsolute()) {
                return info.absoluteFilePath();
            }
            QDir baseDir(settingsDir);
            return baseDir.absoluteFilePath(value);
        };

        qInfo() << "QSslSocket::supportsSsl =" << QSslSocket::supportsSsl();
        qInfo() << "SSL build =" << QSslSocket::sslLibraryBuildVersionString();
        qInfo() << "SSL runtime =" << QSslSocket::sslLibraryVersionString();
        qInfo() << "SSL backends =" << QSslSocket::availableBackends();
        qInfo() << "Active backend =" << QSslSocket::activeBackend();

        bool webhookActive = false;
        if (settings->value("webhook/enabled").toBool()) {
            int portValue = settings->value("webhook/port", 8443).toInt();
            QString certPath = resolvePath(settings->value("webhook/certificatePath").toString());
            QString keyPath = resolvePath(settings->value("webhook/privateKeyPath").toString());
            int maxConnections = settings->value("webhook/maxConnections", 10).toInt();
            if (maxConnections <= 0) {
                maxConnections = 10;
            }

            if (portValue > 0 && portValue <= 0xFFFF && !certPath.isEmpty() && !keyPath.isEmpty()) {
                qDebug() << "Trying to enable webhook on port" << portValue << "using cert" << certPath << "and key" << keyPath;
                webhookActive = bot->setHttpServerWebhook(static_cast<qint16>(portValue), certPath, keyPath, maxConnections,
                                                          TelegramBot::TelegramPollMessageTypes::Message);
                if (!webhookActive) {
                    qWarning() << "Webhook setup failed, falling back to long polling";
                }
            } else {
                qWarning() << "Webhook configuration incomplete or invalid; polling will be used";
            }
        }

        if (!webhookActive) {
            startPolling();
        }

    });


    return a.exec();
}
