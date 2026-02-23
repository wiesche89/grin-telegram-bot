#include "cleanupworker.h"

#include <QDateTime>
#include <QList>
#include <QtGlobal>

#include "account.h"
#include "debugutils.h"
#include "result.h"
#include "txlogentry.h"
#include "walletownerapi.h"

CleanupWorker::CleanupWorker(WalletOwnerApi *walletOwnerApi, QObject *parent) :
    QObject(parent),
    m_walletOwnerApi(walletOwnerApi),
    m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &CleanupWorker::onTimeout);
}

void CleanupWorker::start(int intervalMs)
{
    if (intervalMs <= 0) {
        intervalMs = 5 * 60 * 1000;
    }
    if (m_timer) {
        m_timer->start(intervalMs);
    }
}

void CleanupWorker::triggerCleanup(bool cleanAll)
{
    cleanup(cleanAll);
}

void CleanupWorker::onTimeout()
{
    cleanup(false);
}

void CleanupWorker::cleanup(bool cleanAll)
{
    if (!m_walletOwnerApi) {
        qWarning() << "CleanupWorker: wallet owner API not initialized";
        return;
    }

    QList<Account> accounts;
    Result<QList<Account>> accountResult = m_walletOwnerApi->accounts();
    if (!accountResult.unwrapOrLog(accounts, Q_FUNC_INFO)) {
        qWarning() << "CleanupWorker: failed to list accounts";
        return;
    }

    if (accounts.isEmpty()) {
        qDebug() << "CleanupWorker: no wallet accounts available";
        return;
    }

    for (const Account &account : accounts) {
        const QString label = account.label().trimmed();
        if (label.isEmpty()) {
            continue;
        }
        qDebug() << "CleanupWorker: processing account" << label;
        if (!setActiveAccount(label)) {
            continue;
        }
        cleanupCurrentAccountTransactions(label, cleanAll);
    }
}

bool CleanupWorker::setActiveAccount(const QString &accountLabel)
{
    if (accountLabel.isEmpty()) {
        return false;
    }

    Result<bool> activateResult = m_walletOwnerApi->setActiveAccount(accountLabel);
    bool activated = false;
    if (!activateResult.unwrapOrLog(activated, Q_FUNC_INFO)) {
        qWarning() << "CleanupWorker: failed to activate account" << accountLabel << "-" << activateResult.errorMessage();
        return false;
    }

    if (!activated) {
        qWarning() << "CleanupWorker: setActiveAccount returned false for" << accountLabel;
        return false;
    }

    return true;
}

void CleanupWorker::cleanupCurrentAccountTransactions(const QString &accountLabel, bool cleanAll)
{
    QList<TxLogEntry> txList;
    Result<QList<TxLogEntry>> txResult = m_walletOwnerApi->retrieveTxs(true, 0, "");
    if (!txResult.unwrapOrLog(txList, Q_FUNC_INFO)) {
        qWarning() << "CleanupWorker: failed to retrieve transactions for" << accountLabel;
        return;
    }

    QDateTime now = QDateTime::currentDateTimeUtc();
    for (const TxLogEntry &entry : txList) {
        if (entry.confirmed()) {
            continue;
        }
        if (entry.txType() != "TxReceived" && entry.txType() != "TxSent") {
            continue;
        }

        if (!cleanAll && entry.creationTs().secsTo(now) <= 36000) {
            continue;
        }

        qDebug() << "CleanupWorker: canceling transaction for" << accountLabel << entry.id();
        qInfo().noquote() << debugJsonString(entry);

        Result<bool> cancelResult = m_walletOwnerApi->cancelTx("", entry.id());
        bool canceled = false;
        if (!cancelResult.unwrapOrLog(canceled, Q_FUNC_INFO)) {
            qWarning() << "CleanupWorker: cancelTx failed for" << accountLabel << "-" << cancelResult.errorMessage();
            continue;
        }
        qDebug() << "CleanupWorker: cancelTx result =" << canceled;
    }
}
