#include "tippingdatabase.h"

/**
 * @brief TippingDatabase::TippingDatabase
 * @param dbPath
 * @param parent
 */
TippingDatabase::TippingDatabase(const QString &dbPath, QObject *parent) :
    QObject(parent)
{
    m_connectionName = "tipping_" + QString::number(reinterpret_cast<quintptr>(this));
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);
}

/**
 * @brief TippingDatabase::~TippingDatabase
 */
TippingDatabase::~TippingDatabase()
{
    if (m_query) {
        delete m_query;
        m_query = nullptr;
    }
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

/**
 * @brief TippingDatabase::initialize
 * @return
 */
bool TippingDatabase::initialize()
{
    if (!m_db.open()) {
        qWarning() << "Failed to open tipping database:" << m_db.lastError().text();
        return false;
    }
    else
    {
        qDebug()<<"db connection success!";
    }

    m_query = new QSqlQuery(m_db);
    return ensureTables();
}

/**
 * @brief TippingDatabase::ensureTables
 * @return
 */
bool TippingDatabase::ensureTables()
{
    const char *ledgerSQL = "CREATE TABLE IF NOT EXISTS ledger ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "timestamp INTEGER NOT NULL,"
                            "from_user TEXT,"
                            "to_user TEXT,"
                            "amount INTEGER NOT NULL,"
                            "type TEXT NOT NULL,"
                            "reference TEXT)";
    if (!m_query->exec(ledgerSQL)) {
        qWarning() << "Failed to create ledger table:" << m_query->lastError().text();
        return false;
    }

    const char *balancesSQL = "CREATE TABLE IF NOT EXISTS balances ("
                              "user_id TEXT PRIMARY KEY,"
                              "balance INTEGER NOT NULL)";
    if (!m_query->exec(balancesSQL)) {
        qWarning() << "Failed to create balances table:" << m_query->lastError().text();
        return false;
    }

    const char *usersSQL = "CREATE TABLE IF NOT EXISTS users ("
                           "user_id TEXT PRIMARY KEY,"
                           "username TEXT)";
    if (!m_query->exec(usersSQL)) {
        qWarning() << "Failed to create users table:" << m_query->lastError().text();
        return false;
    }

    const char *pendingSQL = "CREATE TABLE IF NOT EXISTS pending_deposits ("
                             "slate_id TEXT PRIMARY KEY,"
                             "user_id TEXT NOT NULL,"
                             "amount INTEGER NOT NULL,"
                             "chat_id INTEGER,"
                             "first_name TEXT,"
                             "created_at INTEGER NOT NULL,"
                             "completed INTEGER NOT NULL DEFAULT 0)";
    if (!m_query->exec(pendingSQL)) {
        qWarning() << "Failed to create pending deposits table:" << m_query->lastError().text();
        return false;
    }

    // ensure completed column exists for older databases
    m_query->exec("ALTER TABLE pending_deposits ADD COLUMN completed INTEGER NOT NULL DEFAULT 0");

    const char *pendingWithdrawsSQL = "CREATE TABLE IF NOT EXISTS pending_withdrawals ("
                                       "slate_id TEXT PRIMARY KEY,"
                                       "user_id TEXT NOT NULL,"
                                       "amount INTEGER NOT NULL,"
                                       "created_at INTEGER NOT NULL)";
    if (!m_query->exec(pendingWithdrawsSQL)) {
        qWarning() << "Failed to create pending withdrawals table:" << m_query->lastError().text();
        return false;
    }

    const char *pendingWithdrawConfirmationsSQL = "CREATE TABLE IF NOT EXISTS pending_withdraw_confirmations ("
                                                  "slate_id TEXT PRIMARY KEY,"
                                                  "user_id TEXT NOT NULL,"
                                                  "chat_id INTEGER,"
                                                  "first_name TEXT,"
                                                  "amount INTEGER NOT NULL,"
                                                  "created_at INTEGER NOT NULL)";
    if (!m_query->exec(pendingWithdrawConfirmationsSQL)) {
        qWarning() << "Failed to create pending withdraw confirmations table:" << m_query->lastError().text();
        return false;
    }

    return true;
}

/**
 * @brief TippingDatabase::recordTransaction
 * @param fromUser
 * @param toUser
 * @param amount
 * @param type
 * @param reference
 * @return
 */
bool TippingDatabase::recordTransaction(const QString &fromUser, const QString &toUser, qlonglong amount, const QString &type, const QString &reference)
{
    m_query->prepare("INSERT INTO ledger (timestamp, from_user, to_user, amount, type, reference) "
                     "VALUES (?, ?, ?, ?, ?, ?)");
    m_query->addBindValue(QDateTime::currentSecsSinceEpoch());
    m_query->addBindValue(fromUser);
    m_query->addBindValue(toUser);
    m_query->addBindValue(amount);
    m_query->addBindValue(type);
    m_query->addBindValue(reference);
    return m_query->exec();
}

QList<TxLedgerEntry> TippingDatabase::ledgerEntries(int limit)
{
    QList<TxLedgerEntry> entries;
    if (!m_query) return entries;

    m_query->prepare("SELECT timestamp, from_user, to_user, amount, type, reference "
                     "FROM ledger ORDER BY timestamp DESC LIMIT ?");
    m_query->addBindValue(limit);
    if (!m_query->exec()) {
        return entries;
    }

    while (m_query->next()) {
        TxLedgerEntry entry;
        entry.timestamp = m_query->value(0).toLongLong();
        entry.fromUserId = m_query->value(1).toString();
        entry.toUserId = m_query->value(2).toString();
        entry.amount = m_query->value(3).toLongLong();
        entry.type = m_query->value(4).toString();
        entry.reference = m_query->value(5).toString();
        entries.append(entry);
    }
    return entries;
}

bool TippingDatabase::ensureUserRecord(const QString &userId, const QString &username)
{
    if (!m_query || userId.isEmpty() || username.isEmpty()) {
        return false;
    }

    m_query->prepare("REPLACE INTO users (user_id, username) VALUES (?, ?)");
    m_query->addBindValue(userId);
    m_query->addBindValue(username);
    return m_query->exec();
}

QString TippingDatabase::userIdByUsername(const QString &username)
{
    if (!m_query || username.isEmpty()) {
        return {};
    }

    m_query->prepare("SELECT user_id FROM users WHERE lower(username) = lower(?)");
    m_query->addBindValue(username);
    if (m_query->exec() && m_query->next()) {
        return m_query->value(0).toString();
    }

    return {};
}

QString TippingDatabase::usernameByUserId(const QString &userId)
{
    if (!m_query || userId.isEmpty()) {
        return {};
    }

    m_query->prepare("SELECT username FROM users WHERE user_id = ?");
    m_query->addBindValue(userId);
    if (m_query->exec() && m_query->next()) {
        return m_query->value(0).toString();
    }

    return {};
}

/**
 * @brief TippingDatabase::getBalance
 * @param userId
 * @return
 */
qlonglong TippingDatabase::getBalance(const QString &userId)
{
    m_query->prepare("SELECT balance FROM balances WHERE user_id = ?");
    m_query->addBindValue(userId);
    if (m_query->exec() && m_query->next()) {
        return m_query->value(0).toLongLong();
    }
    return 0;
}

/**
 * @brief TippingDatabase::updateBalance
 * @param userId
 * @param amountDelta
 * @return
 */
bool TippingDatabase::updateBalance(const QString &userId, qlonglong amountDelta)
{
    qlonglong current = getBalance(userId);
    qlonglong next = current + amountDelta;
    if (next < 0) {
        return false;
    }
    return setBalance(userId, next);
}

/**
 * @brief TippingDatabase::setBalance
 * @param userId
 * @param balance
 * @return
 */
bool TippingDatabase::setBalance(const QString &userId, qlonglong balance)
{
    m_query->prepare("REPLACE INTO balances (user_id, balance) VALUES (?, ?)");
    m_query->addBindValue(userId);
    m_query->addBindValue(balance);
    return m_query->exec();
}

bool TippingDatabase::insertPendingDeposit(const PendingDepositRecord &deposit)
{
    if (!m_query) return false;
    m_query->prepare("REPLACE INTO pending_deposits (slate_id, user_id, amount, chat_id, first_name, created_at, completed) VALUES (?, ?, ?, ?, ?, ?, ?)");
    m_query->addBindValue(deposit.slateId);
    m_query->addBindValue(deposit.userId);
    m_query->addBindValue(deposit.amount);
    m_query->addBindValue(deposit.chatId);
    m_query->addBindValue(deposit.firstName);
    m_query->addBindValue(QDateTime::currentSecsSinceEpoch());
    m_query->addBindValue(deposit.completed ? 1 : 0);
    return m_query->exec();
}

bool TippingDatabase::removePendingDeposit(const QString &slateId)
{
    if (!m_query) return false;
    m_query->prepare("DELETE FROM pending_deposits WHERE slate_id = ?");
    m_query->addBindValue(slateId);
    return m_query->exec();
}

QList<PendingDepositRecord> TippingDatabase::pendingDeposits()
{
    QList<PendingDepositRecord> list;
    if (!m_query) return list;
    m_query->prepare("SELECT slate_id, user_id, amount, chat_id, first_name, completed FROM pending_deposits WHERE completed = 0");
    if (!m_query->exec()) {
        return list;
    }

    while (m_query->next()) {
        PendingDepositRecord record;
        record.slateId = m_query->value(0).toString();
        record.userId = m_query->value(1).toString();
        record.amount = m_query->value(2).toLongLong();
        record.chatId = m_query->value(3).toLongLong();
        record.firstName = m_query->value(4).toString();
        record.completed = m_query->value(5).toBool();
        list.append(record);
    }
    return list;
}

bool TippingDatabase::pendingDeposit(const QString &slateId, PendingDepositRecord &deposit)
{
    if (!m_query) return false;
    m_query->prepare("SELECT user_id, amount, chat_id, first_name, completed FROM pending_deposits WHERE slate_id = ? AND completed = 0");
    m_query->addBindValue(slateId);
    if (!m_query->exec() || !m_query->next()) {
        return false;
    }
    deposit.slateId = slateId;
    deposit.userId = m_query->value(0).toString();
    deposit.amount = m_query->value(1).toLongLong();
    deposit.chatId = m_query->value(2).toLongLong();
    deposit.firstName = m_query->value(3).toString();
    deposit.completed = m_query->value(4).toBool();
    return true;
}

bool TippingDatabase::markPendingDepositCompleted(const QString &slateId)
{
    if (!m_query) return false;
    m_query->prepare("UPDATE pending_deposits SET completed = 1 WHERE slate_id = ?");
    m_query->addBindValue(slateId);
    return m_query->exec();
}

bool TippingDatabase::insertPendingWithdraw(const PendingWithdrawRecord &withdraw)
{
    if (!m_query) return false;
    m_query->prepare("REPLACE INTO pending_withdrawals (slate_id, user_id, amount, created_at) VALUES (?, ?, ?, ?)");
    m_query->addBindValue(withdraw.slateId);
    m_query->addBindValue(withdraw.userId);
    m_query->addBindValue(withdraw.amount);
    m_query->addBindValue(withdraw.createdAt);
    return m_query->exec();
}

bool TippingDatabase::removePendingWithdraw(const QString &slateId)
{
    if (!m_query) return false;
    m_query->prepare("DELETE FROM pending_withdrawals WHERE slate_id = ?");
    m_query->addBindValue(slateId);
    return m_query->exec();
}

QList<PendingWithdrawRecord> TippingDatabase::pendingWithdrawals()
{
    QList<PendingWithdrawRecord> list;
    if (!m_query) return list;
    m_query->prepare("SELECT slate_id, user_id, amount, created_at FROM pending_withdrawals");
    if (!m_query->exec()) {
        return list;
    }

    while (m_query->next()) {
        PendingWithdrawRecord record;
        record.slateId = m_query->value(0).toString();
        record.userId = m_query->value(1).toString();
        record.amount = m_query->value(2).toLongLong();
        record.createdAt = m_query->value(3).toLongLong();
        list.append(record);
    }
    return list;
}

bool TippingDatabase::pendingWithdraw(const QString &slateId, PendingWithdrawRecord &withdraw)
{
    if (!m_query) return false;
    m_query->prepare("SELECT user_id, amount, created_at FROM pending_withdrawals WHERE slate_id = ?");
    m_query->addBindValue(slateId);
    if (!m_query->exec() || !m_query->next()) {
        return false;
    }
    withdraw.slateId = slateId;
    withdraw.userId = m_query->value(0).toString();
    withdraw.amount = m_query->value(1).toLongLong();
    withdraw.createdAt = m_query->value(2).toLongLong();
    return true;
}

bool TippingDatabase::insertPendingWithdrawConfirmation(const PendingWithdrawConfirmationRecord &confirmation)
{
    if (!m_query) return false;
    m_query->prepare("REPLACE INTO pending_withdraw_confirmations (slate_id, user_id, chat_id, first_name, amount, created_at) VALUES (?, ?, ?, ?, ?, ?)");
    m_query->addBindValue(confirmation.slateId);
    m_query->addBindValue(confirmation.userId);
    m_query->addBindValue(confirmation.chatId);
    m_query->addBindValue(confirmation.firstName);
    m_query->addBindValue(confirmation.amount);
    m_query->addBindValue(confirmation.createdAt);
    return m_query->exec();
}

bool TippingDatabase::removePendingWithdrawConfirmation(const QString &slateId)
{
    if (!m_query) return false;
    m_query->prepare("DELETE FROM pending_withdraw_confirmations WHERE slate_id = ?");
    m_query->addBindValue(slateId);
    return m_query->exec();
}

QList<PendingWithdrawConfirmationRecord> TippingDatabase::pendingWithdrawConfirmations()
{
    QList<PendingWithdrawConfirmationRecord> list;
    if (!m_query) return list;
    m_query->prepare("SELECT slate_id, user_id, chat_id, first_name, amount, created_at FROM pending_withdraw_confirmations");
    if (!m_query->exec()) {
        return list;
    }

    while (m_query->next()) {
        PendingWithdrawConfirmationRecord record;
        record.slateId = m_query->value(0).toString();
        record.userId = m_query->value(1).toString();
        record.chatId = m_query->value(2).toLongLong();
        record.firstName = m_query->value(3).toString();
        record.amount = m_query->value(4).toLongLong();
        record.createdAt = m_query->value(5).toLongLong();
        list.append(record);
    }
    return list;
}

QList<BalanceRecord> TippingDatabase::listBalances()
{
    QList<BalanceRecord> list;
    if (!m_query) return list;

    m_query->prepare("SELECT user_id, balance FROM balances ORDER BY balance DESC, user_id ASC");
    if (!m_query->exec()) {
        return list;
    }

    while (m_query->next()) {
        BalanceRecord record;
        record.userId = m_query->value(0).toString();
        record.balance = m_query->value(1).toLongLong();
        list.append(record);
    }
    return list;
}
