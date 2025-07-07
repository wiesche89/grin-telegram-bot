#include "databasemanager.h"

/**
 * @brief DatabaseManager::DatabaseManager
 * @param parent
 */
DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
}

/**
 * @brief DatabaseManager::~DatabaseManager
 */
DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

/**
 * @brief DatabaseManager::connectToDatabase
 * @param dbPath
 * @return
 */
bool DatabaseManager::connectToDatabase(const QString &dbPath)
{
    if (db.isOpen()) {
        db.close();
    }

    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCritical() << "Datenbank could not be open:" << db.lastError().text();
        return false;
    }

    qDebug() << "Database connection:" << dbPath;
    return true;
}

/**
 * @brief DatabaseManager::closeDatabase
 */
void DatabaseManager::closeDatabase()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database close.";
    }
}

/**
 * @brief DatabaseManager::getDatabase
 * @return
 */
QSqlDatabase DatabaseManager::getDatabase() const
{
    return db;
}

// ------------------ Donate CRUD ------------------
/**
 * @brief DatabaseManager::insertDonate
 * @param donate
 * @return
 */
bool DatabaseManager::insertDonate(const Donate &donate)
{
    QSqlQuery query;
    query.prepare("INSERT INTO DONATE (UserId, Username, Amount, Date) VALUES (?, ?, ?, ?)");
    query.addBindValue(donate.userId());
    query.addBindValue(donate.username());
    query.addBindValue(donate.amount());
    query.addBindValue(donate.date());

    query.exec();

    if (query.lastError().isValid()) {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError().text();
        return false;
    }

    return true;
}

/**
 * @brief DatabaseManager::getDonateById
 * @param id
 * @return
 */
Donate DatabaseManager::getDonateById(int id)
{
    QSqlQuery query;
    query.prepare("SELECT Id, UserId, Username, Amount, Date FROM DONATE WHERE Id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        Donate d = Donate(query.value(0).toInt(),
                          query.value(1).toString(),
                          query.value(2).toString(),
                          query.value(3).toString(),
                          query.value(4).toString());

        return d;
    }
    return Donate();
}

QList<Donate> DatabaseManager::getAllDonate()
{
    QList<Donate> list;
    QSqlQuery query;

    query.exec("SELECT Id, UserId, Username, Amount, Date FROM DONATE");

    while (query.next()) {
        Donate d = Donate(query.value(0).toInt(),
                          query.value(1).toString(),
                          query.value(2).toString(),
                          query.value(3).toString(),
                          query.value(4).toString());

        list.append(d);
    }
    return list;
}

/**
 * @brief DatabaseManager::updateDonate
 * @param donate
 * @return
 */
bool DatabaseManager::updateDonate(const Donate &donate)
{
    QSqlQuery query;
    query.prepare("UPDATE DONATE SET UserId = ?, Username = ?, Amount = ?, Date = ? WHERE Id = ?");
    query.addBindValue(donate.userId());
    query.addBindValue(donate.username());
    query.addBindValue(donate.amount());
    query.addBindValue(donate.date());
    query.addBindValue(donate.id());
    return query.exec();
}

/**
 * @brief DatabaseManager::deleteDonate
 * @param id
 * @return
 */
bool DatabaseManager::deleteDonate(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM DONATE WHERE Id = ?");
    query.addBindValue(id);
    return query.exec();
}

// ------------------ Faucet CRUD ------------------
/**
 * @brief DatabaseManager::insertFaucet
 * @param faucet
 * @return
 */
bool DatabaseManager::insertFaucet(const Faucet &faucet)
{
    QSqlQuery query;
    query.prepare("INSERT INTO FAUCET (UserId, Username, Amount, Date) VALUES (?, ?, ?, ?)");
    query.addBindValue(faucet.userId());
    query.addBindValue(faucet.username());
    query.addBindValue(faucet.amount());
    query.addBindValue(faucet.date());
    query.exec();

    if (query.lastError().isValid()) {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError().text();
        return false;
    }

    return true;
}

/**
 * @brief DatabaseManager::getFaucetById
 * @param id
 * @return
 */
Faucet DatabaseManager::getFaucetById(int id)
{
    QSqlQuery query;
    query.prepare("SELECT Id, UserId, Username, Amount, Date FROM FAUCET WHERE Id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        return Faucet(query.value(0).toInt(),
                      query.value(1).toString(),
                      query.value(2).toString(),
                      query.value(3).toString(),
                      query.value(4).toString());
    }
    return Faucet();
}

/**
 * @brief DatabaseManager::updateFaucet
 * @param faucet
 * @return
 */
bool DatabaseManager::updateFaucet(const Faucet &faucet)
{
    QSqlQuery query;
    query.prepare("UPDATE FAUCET SET UserId = ?, Username = ?, Amount = ?, Date = ? WHERE Id = ?");
    query.addBindValue(faucet.userId());
    query.addBindValue(faucet.username());
    query.addBindValue(faucet.amount());
    query.addBindValue(faucet.date());
    query.addBindValue(faucet.id());
    return query.exec();
}

/**
 * @brief DatabaseManager::deleteFaucet
 * @param id
 * @return
 */
bool DatabaseManager::deleteFaucet(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM FAUCET WHERE Id = ?");
    query.addBindValue(id);
    return query.exec();
}

/**
 * @brief DatabaseManager::getFaucetAmountForToday
 * @param userId
 * @return
 */
QString DatabaseManager::getFaucetAmountForToday(const QString &userId)
{
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    QSqlQuery query;
    query.prepare(R"(
        SELECT IFNULL(SUM(CAST(Amount AS REAL)), 0)
        FROM FAUCET
        WHERE UserId = ? AND Date = ?
    )");
    query.addBindValue(userId);
    query.addBindValue(today);

    if (query.exec() && query.next()) {
        qlonglong total = query.value(0).toDouble();
        return QString::number(total, 'f', 0);

        return query.value(0).toString();
    }

    return "0";
}

/**
 * @brief DatabaseManager::getAllFaucetAmountForToday
 * @return
 */
QList<Faucet> DatabaseManager::getAllFaucetAmountForToday()
{
    QList<Faucet> list;
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    QSqlQuery query;
    query.prepare(R"(SELECT Id, UserId, Username, Amount, Date FROM FAUCET WHERE Date = ?)");
    query.addBindValue(today);
    query.exec();

    while (query.next())
    {
        Faucet f = Faucet(query.value(0).toInt(),
                          query.value(1).toString(),
                          query.value(2).toString(),
                          query.value(3).toString(),
                          query.value(4).toString());

        list.append(f);
    }

    return list;
}

QList<Faucet> DatabaseManager::getAllFaucet()
{
    QList<Faucet> list;

    QSqlQuery query;
    query.exec(R"(SELECT Id, UserId, Username, Amount, Date FROM FAUCET)");

    while (query.next())
    {
        Faucet f = Faucet(query.value(0).toInt(),
                          query.value(1).toString(),
                          query.value(2).toString(),
                          query.value(3).toString(),
                          query.value(4).toString());

        list.append(f);
    }

    return list;
}
