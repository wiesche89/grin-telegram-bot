#include "grinwalletmanager.h"

/**
 * @brief GrinWalletManager::GrinWalletManager
 * @param parent
 */
GrinWalletManager::GrinWalletManager(QObject *parent) :
    QObject(parent),
    m_walletProcess(new QProcess(this)),
    //m_jobHandle(nullptr),
    m_pid(-1)
{
    setupJobObject();

    connect(qApp, &QCoreApplication::aboutToQuit, this, &GrinWalletManager::stopWallet);

    connect(m_walletProcess, &QProcess::readyReadStandardOutput, [this]() {
        QStringList stdOut = QString::fromUtf8(m_walletProcess->readAllStandardOutput()).split("\r\n");

        for (int i = 0; i < stdOut.length(); i++) {
            qDebug() << stdOut[i];
        }
    });

    connect(m_walletProcess, &QProcess::readyReadStandardError, [this]() {
        QStringList stdOut = QString::fromUtf8(m_walletProcess->readAllStandardOutput()).split("\r\n");

        for (int i = 0; i < stdOut.length(); i++) {
            qDebug() << stdOut[i];
        }
    });
}

/**
 * @brief GrinWalletManager::~GrinWalletManager
 */
GrinWalletManager::~GrinWalletManager()
{
    stopWallet();
//    if (m_jobHandle) {
//        CloseHandle(m_jobHandle);
//        m_jobHandle = nullptr;
//    }
}

/**
 * @brief GrinWalletManager::setupJobObject
 */
void GrinWalletManager::setupJobObject()
{
//    m_jobHandle = CreateJobObject(nullptr, nullptr);
//    if (m_jobHandle == nullptr) {
//        qWarning() << "CreateJobObject failed:" << GetLastError();
//        return;
//    }

    // JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE causes all processes in the job to be terminated when the job handle is closed
//    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
//    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

//    if (!SetInformationJobObject(m_jobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
//        qWarning() << "SetInformationJobObject failed:" << GetLastError();
//        CloseHandle(m_jobHandle);
//        m_jobHandle = nullptr;
//    }
}

/**
 * @brief GrinWalletManager::startWallet
 */
bool GrinWalletManager::startWallet()
{
    if (isWalletRunning()) {
        qCritical() << "Wallet process already running.";
        return false;
    }

    m_walletProcess->start("grin-wallet", {"owner_api", "--run_foreign"});

    if (!m_walletProcess->waitForStarted(3000)) {
        qCritical() << "Error: grin-wallet process could not be started.";
        return false;
    }

    // Kindprozess dem Job Object zuweisen
//    HANDLE processHandle = (HANDLE)m_walletProcess->processId();
//    if (m_jobHandle && processHandle) {
//        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_walletProcess->processId());
//        if (hProcess) {
//            if (!AssignProcessToJobObject(m_jobHandle, hProcess)) {
//                qWarning() << "AssignProcessToJobObject failed:" << GetLastError();
//            }
//            CloseHandle(hProcess);
//        } else {
//            qWarning() << "OpenProcess failed:" << GetLastError();
//        }
//    }

    m_pid = m_walletProcess->processId();
    qDebug() << "grin-wallet started, PID:" << m_pid;

    QTimer *monitorTimer = new QTimer(this);
    QObject::connect(monitorTimer, &QTimer::timeout, [&]() {
        if (m_walletProcess->processId() != m_pid) {
            qDebug() << "grin-wallet-process was terminated.";
            monitorTimer->stop();
            QCoreApplication::quit();
        }
    });
    monitorTimer->start(1000);

    return true;
}

/**
 * @brief GrinWalletManager::stopWallet
 */
void GrinWalletManager::stopWallet()
{
    if (!isWalletRunning()) {
        return;
    }

    qDebug() << "Close grin-wallet...";

    m_walletProcess->terminate();
    if (!m_walletProcess->waitForFinished(3000)) {
        qDebug() << "Process does not respond, force kill.";
        m_walletProcess->kill();
        m_walletProcess->waitForFinished(3000);
    }
}

/**
 * @brief GrinWalletManager::isWalletRunning
 * @return
 */
bool GrinWalletManager::isWalletRunning() const
{
    return m_walletProcess->state() != QProcess::NotRunning;
}
