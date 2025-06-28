#ifndef GRINWALLETMANAGER_H
#define GRINWALLETMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
//#include <windows.h>

class GrinWalletManager : public QObject
{
    Q_OBJECT
public:
    explicit GrinWalletManager(QObject *parent = nullptr);
    ~GrinWalletManager();

    bool startWallet();
    void stopWallet();
    bool isWalletRunning() const;

private:
    void setupJobObject();

    QProcess *m_walletProcess;
    //HANDLE m_jobHandle;
    int m_pid;
};

#endif // GRINWALLETMANAGER_H
