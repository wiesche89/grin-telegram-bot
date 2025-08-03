#include "dexdatabase.h"

DexDatabase::DexDatabase(const QString &path, QObject *parent)
    : QObject(parent),
    m_connectionName("dex_connection")
{
    if (QSqlDatabase::contains(m_connectionName)) {
        m_db = QSqlDatabase::database(m_connectionName);
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        m_db.setDatabaseName(path);
    }
}

bool DexDatabase::initialize()
{
    if (!m_db.open()) {
        qWarning() << "[DB] Cannot open database:" << m_db.lastError().text();
        return false;
    }
    return ensureTables();
}

bool DexDatabase::ensureTables()
{
    QSqlQuery q(m_db);

    const char *balancesSQL =
        "CREATE TABLE IF NOT EXISTS balances ("
        "user_id TEXT PRIMARY KEY,"
        "grin_balance INTEGER NOT NULL DEFAULT 0,"
        "btc_balance INTEGER NOT NULL DEFAULT 0"
        ");";

    const char *ledgerSQL =
        "CREATE TABLE IF NOT EXISTS ledger ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp INTEGER NOT NULL,"
        "user_id TEXT NOT NULL,"
        "type TEXT NOT NULL,"  // deposit, withdraw, trade
        "asset TEXT NOT NULL," // grin or btc
        "amount INTEGER NOT NULL,"
        "reference TEXT"
        ");";

    const char *ordersSQL =
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id TEXT NOT NULL,"
        "type TEXT NOT NULL,"          // buy or sell
        "base TEXT NOT NULL,"          // grin
        "quote TEXT NOT NULL,"         // btc
        "amount INTEGER NOT NULL,"
        "price INTEGER NOT NULL,"
        "status TEXT NOT NULL,"        // open, matched, canceled
        "created_at INTEGER NOT NULL"
        ");";

    const char *tradesSQL =
        "CREATE TABLE IF NOT EXISTS trades ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "buy_order_id INTEGER NOT NULL,"
        "sell_order_id INTEGER NOT NULL,"
        "price INTEGER NOT NULL,"
        "amount INTEGER NOT NULL,"
        "timestamp INTEGER NOT NULL"
        ");";

    return q.exec(balancesSQL)
           && q.exec(ledgerSQL)
           && q.exec(ordersSQL)
           && q.exec(tradesSQL);
}

QSqlDatabase DexDatabase::database() const
{
    return m_db;
}
