#include <QCoreApplication>
#include <QProcess>

#include "worker.h"
#include "grinwalletmanager.h"

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Manager
    GrinWalletManager manager;
    if (!manager.startWallet()) {
        return 0;
    }

    // Start worker, wait x msecs
    QTimer::singleShot(3000, [&]() {
        qDebug()<<"start worker";
        Worker *worker = new Worker();
        if (!worker->init()) {
            QCoreApplication::quit();
        }
    });

    return a.exec();
}
