#ifndef TIPPINGDATABASE_H
#define TIPPINGDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QList>

struct PendingDepositRecord
{
    QString slateId;
    QString userId;
    qlonglong chatId;
    QString firstName;
    qlonglong amount;
    bool completed = false;
};

struct PendingWithdrawRecord
{
    QString slateId;
    QString userId;
    qlonglong amount;
    qlonglong createdAt;
};

struct PendingWithdrawConfirmationRecord
{
    QString slateId;
    QString userId;
    qlonglong chatId;
    QString firstName;
    qlonglong amount;
    qlonglong createdAt;
};

struct TxLedgerEntry
{
    qlonglong timestamp;
    QString fromUserId;
    QString toUserId;
    qlonglong amount;
    QString type;
    QString reference;
};

struct BalanceRecord
{
    QString userId;
    qlonglong balance;
};

class TippingDatabase : public QObject
{
    Q_OBJECT

public:
    explicit TippingDatabase(const QString &dbPath, QObject *parent = nullptr);
    ~TippingDatabase();

    bool initialize();
    bool recordTransaction(const QString &fromUser, const QString &toUser, qlonglong amount, const QString &type, const QString &reference = QString());
    qlonglong getBalance(const QString &userId);
    bool updateBalance(const QString &userId, qlonglong amountDelta);
    bool setBalance(const QString &userId, qlonglong balance);
    bool insertPendingDeposit(const PendingDepositRecord &deposit);
    bool removePendingDeposit(const QString &slateId);
    QList<PendingDepositRecord> pendingDeposits();
    bool pendingDeposit(const QString &slateId, PendingDepositRecord &deposit);
    bool markPendingDepositCompleted(const QString &slateId);

    bool insertPendingWithdraw(const PendingWithdrawRecord &withdraw);
    bool removePendingWithdraw(const QString &slateId);
    QList<PendingWithdrawRecord> pendingWithdrawals();
    bool pendingWithdraw(const QString &slateId, PendingWithdrawRecord &withdraw);

    bool insertPendingWithdrawConfirmation(const PendingWithdrawConfirmationRecord &confirmation);
    bool removePendingWithdrawConfirmation(const QString &slateId);
    QList<PendingWithdrawConfirmationRecord> pendingWithdrawConfirmations();

    QList<TxLedgerEntry> ledgerEntries(int limit = 20);

    bool ensureUserRecord(const QString &userId, const QString &username);
    QString userIdByUsername(const QString &username);
    QString usernameByUserId(const QString &userId);
    QList<BalanceRecord> listBalances();

private:
    QSqlDatabase m_db;
    QSqlQuery *m_query = nullptr;
    QString m_connectionName;

    bool ensureTables();
};

#endif // TIPPINGDATABASE_H
