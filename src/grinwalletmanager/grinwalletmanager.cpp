#include "grinwalletmanager.h"

/**
 * @brief GrinWalletManager::GrinWalletManager
 * @param parent
 */
GrinWalletManager::GrinWalletManager(QObject *parent) :
    QObject(parent),
    m_walletProcess(new QProcess(this)),
    m_pid(-1)
{
    #ifdef Q_OS_WIN
    m_jobHandle = nullptr;
    #endif

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

    #ifdef Q_OS_WIN
    if (m_jobHandle) {
        CloseHandle(m_jobHandle);
        m_jobHandle = nullptr;
    }
    #elif defined(Q_OS_LINUX)
    // Unter Linux ist kein Handle zu schließen
    qInfo() << "No job object cleanup required on Linux.";
    #endif
}

/**
 * @brief GrinWalletManager::setupJobObject
 */
void GrinWalletManager::setupJobObject()
{
#ifdef Q_OS_WIN
    m_jobHandle = CreateJobObject(nullptr, nullptr);
    if (m_jobHandle == nullptr) {
        qWarning() << "CreateJobObject failed:" << GetLastError();
        return;
    }

    // Set up the job object to kill all child processes when the job is closed
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if (!SetInformationJobObject(m_jobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        qWarning() << "SetInformationJobObject failed:" << GetLastError();
        CloseHandle(m_jobHandle);
        m_jobHandle = nullptr;
    }
#elif defined(Q_OS_LINUX)
    // Linux: nothing to initialize here, but we can log it
    qInfo() << "Job object setup not required on Linux.";
#endif
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

    QString program;

    #ifdef Q_OS_WIN
    program = "grin-wallet";
    #else
    program = "./grin-wallet";
    #endif

    QString net = qEnvironmentVariable("GRIN_CHAIN_TYPE");

    qDebug()<<"net: "<<net;

    if(net == "testnet")
    {
        m_walletProcess->start(program, {"--testnet","owner_api", "--run_foreign"});
    }
    else
    {
        m_walletProcess->start(program, {"owner_api", "--run_foreign"});
    }

    if (!m_walletProcess->waitForStarted(3000)) {
        qCritical() << "Error: grin-wallet process could not be started.";
        return false;
    }

        qDebug()<<"waitForStarted success ";

#ifdef Q_OS_WIN
    HANDLE processHandle = (HANDLE)m_walletProcess->processId();
    if (m_jobHandle && processHandle) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_walletProcess->processId());
        if (hProcess) {
            if (!AssignProcessToJobObject(m_jobHandle, hProcess)) {
                qWarning() << "AssignProcessToJobObject failed:" << GetLastError();
            }
            CloseHandle(hProcess);
        } else {
            qWarning() << "OpenProcess failed:" << GetLastError();
        }
    }
#elif defined(Q_OS_LINUX)
    pid_t pid = m_walletProcess->processId();
    if (pid > 0) {
        // Set child to new process group so it can be killed later with killpg
        if (setpgid(pid, pid) != 0) {
            perror("setpgid failed");
        }
        // Optional: ensure child dies with parent
        if (prctl(PR_SET_PDEATHSIG, SIGTERM) != 0) {
            perror("prctl(PR_SET_PDEATHSIG) failed");
        }
    }
#endif

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
