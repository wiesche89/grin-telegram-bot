#ifndef GRINWALLETMANAGER_H
#define GRINWALLETMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>
#endif

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
    void logProcessOutput(const QByteArray &data);
    void setupJobObject();

private slots:
    void handleWalletStandardOutput();
    void handleWalletStandardError();
    void monitorWalletProcess();

private:
    QProcess *m_walletProcess;

    #ifdef Q_OS_WIN
    HANDLE m_jobHandle;
    #endif

    int m_pid;
    QTimer *m_monitorTimer;
};

#endif // GRINWALLETMANAGER_H
