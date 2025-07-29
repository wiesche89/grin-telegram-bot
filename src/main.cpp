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
    QApplication a(argc, argv);

    // Manager
    GrinWalletManager manager;
    if (!manager.startWallet()) {
        return 0;
    }

    // Start worker, wait x msecs
    QTimer::singleShot(3000, [&]() {
        qDebug() << "start worker";

        // Bot - Instance
        QSettings *settings = new QSettings(QCoreApplication::applicationDirPath() + "/etc/settings.ini", QSettings::IniFormat);

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
        tip - Tipping Grins to other user
        blackjack - Start BlackJack game with other user

        Tradeogre commands
        */
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

        // TippingWorker *tippingWorker = new TippingWorker(bot,settings);
        // if (!tippingWorker->init()) {
        //     qDebug()<<"Tipping Worker init failed!";
        //     QCoreApplication::quit();
        // }


        bot->startMessagePulling();

    });


    return a.exec();
}
