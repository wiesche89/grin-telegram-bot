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

    const char *pendingSQL = "CREATE TABLE IF NOT EXISTS pending_deposits ("
                             "slate_id TEXT PRIMARY KEY,"
                             "user_id TEXT NOT NULL,"
                             "amount INTEGER NOT NULL,"
                             "chat_id INTEGER,"
                             "first_name TEXT,"
                             "created_at INTEGER NOT NULL)";
    if (!m_query->exec(pendingSQL)) {
        qWarning() << "Failed to create pending deposits table:" << m_query->lastError().text();
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
bool TippingDatabase::recordTransaction(const QString &fromUser, const QString &toUser, int amount, const QString &type, const QString &reference)
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

/**
 * @brief TippingDatabase::getBalance
 * @param userId
 * @return
 */
int TippingDatabase::getBalance(const QString &userId)
{
    m_query->prepare("SELECT balance FROM balances WHERE user_id = ?");
    m_query->addBindValue(userId);
    if (m_query->exec() && m_query->next()) {
        return m_query->value(0).toInt();
    }
    return 0;
}

/**
 * @brief TippingDatabase::updateBalance
 * @param userId
 * @param amountDelta
 * @return
 */
bool TippingDatabase::updateBalance(const QString &userId, int amountDelta)
{
    int current = getBalance(userId);
    return setBalance(userId, current + amountDelta);
}

/**
 * @brief TippingDatabase::setBalance
 * @param userId
 * @param balance
 * @return
 */
bool TippingDatabase::setBalance(const QString &userId, int balance)
{
    m_query->prepare("REPLACE INTO balances (user_id, balance) VALUES (?, ?)");
    m_query->addBindValue(userId);
    m_query->addBindValue(balance);
    return m_query->exec();
}

bool TippingDatabase::insertPendingDeposit(const PendingDepositRecord &deposit)
{
    if (!m_query) return false;
    m_query->prepare("REPLACE INTO pending_deposits (slate_id, user_id, amount, chat_id, first_name, created_at) VALUES (?, ?, ?, ?, ?, ?)");
    m_query->addBindValue(deposit.slateId);
    m_query->addBindValue(deposit.userId);
    m_query->addBindValue(deposit.amount);
    m_query->addBindValue(deposit.chatId);
    m_query->addBindValue(deposit.firstName);
    m_query->addBindValue(QDateTime::currentSecsSinceEpoch());
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
    m_query->prepare("SELECT slate_id, user_id, amount, chat_id, first_name FROM pending_deposits");
    if (!m_query->exec()) {
        return list;
    }

    while (m_query->next()) {
        PendingDepositRecord record;
        record.slateId = m_query->value(0).toString();
        record.userId = m_query->value(1).toString();
        record.amount = m_query->value(2).toInt();
        record.chatId = m_query->value(3).toLongLong();
        record.firstName = m_query->value(4).toString();
        list.append(record);
    }
    return list;
}

bool TippingDatabase::pendingDeposit(const QString &slateId, PendingDepositRecord &deposit)
{
    if (!m_query) return false;
    m_query->prepare("SELECT user_id, amount, chat_id, first_name FROM pending_deposits WHERE slate_id = ?");
    m_query->addBindValue(slateId);
    if (!m_query->exec() || !m_query->next()) {
        return false;
    }
    deposit.slateId = slateId;
    deposit.userId = m_query->value(0).toString();
    deposit.amount = m_query->value(1).toInt();
    deposit.chatId = m_query->value(2).toLongLong();
    deposit.firstName = m_query->value(3).toString();
    return true;
}
