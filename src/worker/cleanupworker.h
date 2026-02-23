#ifndef CLEANUPWORKER_H
#define CLEANUPWORKER_H

#include <QObject>
#include <QTimer>
#include <QString>

class WalletOwnerApi;

class CleanupWorker : public QObject
{
    Q_OBJECT

public:
    explicit CleanupWorker(WalletOwnerApi *walletOwnerApi, QObject *parent = nullptr);
    void start(int intervalMs = 5 * 60 * 1000);
    void triggerCleanup(bool cleanAll = false);

private slots:
    void onTimeout();

private:
    void cleanup(bool cleanAll);
    bool setActiveAccount(const QString &accountLabel);
    void cleanupCurrentAccountTransactions(const QString &accountLabel, bool cleanAll);

    WalletOwnerApi *m_walletOwnerApi;
    QTimer *m_timer;
};

#endif // CLEANUPWORKER_H
