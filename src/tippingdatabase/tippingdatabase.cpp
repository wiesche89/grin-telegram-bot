#include "tippingdatabase.h"

TippingDatabase::TippingDatabase(const QString &dbPath, QObject *parent) :
    QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
}

bool TippingDatabase::initialize()
{
    if (!m_db.open()) {
        qWarning() << "Failed to open tipping database:" << m_db.lastError().text();
        return false;
    }
    return ensureTables();
}

bool TippingDatabase::ensureTables()
{
    QSqlQuery query;

    const char *ledgerSQL = "CREATE TABLE IF NOT EXISTS ledger ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "timestamp INTEGER NOT NULL,"
                            "from_user TEXT,"
                            "to_user TEXT,"
                            "amount INTEGER NOT NULL,"
                            "type TEXT NOT NULL,"
                            "reference TEXT)";
    if (!query.exec(ledgerSQL)) {
        return false;
    }

    const char *balancesSQL = "CREATE TABLE IF NOT EXISTS balances ("
                              "user_id TEXT PRIMARY KEY,"
                              "balance INTEGER NOT NULL)";
    return query.exec(balancesSQL);
}

bool TippingDatabase::recordTransaction(const QString &fromUser, const QString &toUser, int amount, const QString &type,
                                        const QString &reference)
{
    QSqlQuery query;
    query.prepare("INSERT INTO ledger (timestamp, from_user, to_user, amount, type, reference) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(fromUser);
    query.addBindValue(toUser);
    query.addBindValue(amount);
    query.addBindValue(type);
    query.addBindValue(reference);
    return query.exec();
}

int TippingDatabase::getBalance(const QString &userId)
{
    QSqlQuery query;
    query.prepare("SELECT balance FROM balances WHERE user_id = ?");
    query.addBindValue(userId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool TippingDatabase::updateBalance(const QString &userId, int amountDelta)
{
    int current = getBalance(userId);
    return setBalance(userId, current + amountDelta);
}

bool TippingDatabase::setBalance(const QString &userId, int balance)
{
    QSqlQuery query;
    query.prepare("REPLACE INTO balances (user_id, balance) VALUES (?, ?)");
    query.addBindValue(userId);
    query.addBindValue(balance);
    return query.exec();
}
