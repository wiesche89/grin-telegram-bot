#include <QApplication>
#include <QProcess>

#include "grinwalletmanager.h"
#include "ggcworker.h"
#include "tippingworker.h"
#include "tradeogreworker.h"

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
        GGC commands
        start - introduction
        address - get slatepack address
        donate - dm slatepack address
        faucet - use faucet
        rewindhash - get rewindhash
        scanrewindhash - scan current rewindhash

        Tradeogre commands
        price - TO price USDT and BTC
        orderbook - TO orderbook
        chart - TO chart 4h BTC and USDT
        history - TO last x trades BTC and USDT

        Admin commands
        adminenabledisabledeposits - enable/disable deposits
        adminenabledisablewithdrawals - enable/disable withdrawals
        adminupdateresponsemessage - update bot response messages templates
        adminrequirednumberofresponse - set the required number of responses to approve the withdrawal
        adminprofilrequirementswithdrawl - set the profile requirements to approve the withdrawal
        adminapprovedwithdrawalamount - set the approved withdrawal amount
        adminamount - get account amounts

        Tipping commands
        deposit - example /deposit 10
        withdraw - example /withdraw 10
        tip - tipping Grins to other user example /tip @user 10
        blackjack - start BlackJack game with other user /blackjack @user 10
        hit - command blackjack
        stand - command blackjack
        balance - shows your balance
        cancelgame - cancel current game
        games - list all active games
        */

        // Bot - Instance
        TelegramBot *bot = new TelegramBot(settings->value("bot/token").toString());

        GgcWorker *ggcWorker = new GgcWorker(bot,settings);
        if (!ggcWorker->init()) {
            qDebug()<<"GGC Worker init failed!";
            QCoreApplication::quit();
        }

        TradeOgreWorker *tradeOgreWorker = new TradeOgreWorker(bot,settings);
        if (!tradeOgreWorker->init()) {
            qDebug()<<"Tradeogre Worker init failed!";
            QCoreApplication::quit();
        }

        TippingWorker *tippingWorker = new TippingWorker(bot,settings);
        if (!tippingWorker->init()) {
            qDebug()<<"Tipping Worker init failed!";
            QCoreApplication::quit();
        }

        bot->startMessagePulling();

    });


    return a.exec();
}
