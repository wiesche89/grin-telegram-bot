#include "ggcdatabasemanager.h"

/**
 * @brief GgcDatabaseManager::GgcDatabaseManager
 * @param parent
 */
GgcDatabaseManager::GgcDatabaseManager(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
}

/**
 * @brief GgcDatabaseManager::~GgcDatabaseManager
 */
GgcDatabaseManager::~GgcDatabaseManager()
{
    closeDatabase();
}

/**
 * @brief GgcDatabaseManager::connectToDatabase
 * @param dbPath
 * @return
 */
bool GgcDatabaseManager::connectToDatabase(const QString &dbPath)
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
 * @brief GgcDatabaseManager::closeDatabase
 */
void GgcDatabaseManager::closeDatabase()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database close.";
    }
}

/**
 * @brief GgcDatabaseManager::getDatabase
 * @return
 */
QSqlDatabase GgcDatabaseManager::getDatabase() const
{
    return db;
}

// ------------------ Donate CRUD ------------------
/**
 * @brief GgcDatabaseManager::insertDonate
 * @param donate
 * @return
 */
bool GgcDatabaseManager::insertDonate(const Donate &donate)
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
 * @brief GgcDatabaseManager::getDonateById
 * @param id
 * @return
 */
Donate GgcDatabaseManager::getDonateById(int id)
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

QList<Donate> GgcDatabaseManager::getAllDonate()
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
 * @brief GgcDatabaseManager::updateDonate
 * @param donate
 * @return
 */
bool GgcDatabaseManager::updateDonate(const Donate &donate)
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
 * @brief GgcDatabaseManager::deleteDonate
 * @param id
 * @return
 */
bool GgcDatabaseManager::deleteDonate(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM DONATE WHERE Id = ?");
    query.addBindValue(id);
    return query.exec();
}

// ------------------ Faucet CRUD ------------------
/**
 * @brief GgcDatabaseManager::insertFaucet
 * @param faucet
 * @return
 */
bool GgcDatabaseManager::insertFaucet(const Faucet &faucet)
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
 * @brief GgcDatabaseManager::getFaucetById
 * @param id
 * @return
 */
Faucet GgcDatabaseManager::getFaucetById(int id)
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
 * @brief GgcDatabaseManager::updateFaucet
 * @param faucet
 * @return
 */
bool GgcDatabaseManager::updateFaucet(const Faucet &faucet)
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
 * @brief GgcDatabaseManager::deleteFaucet
 * @param id
 * @return
 */
bool GgcDatabaseManager::deleteFaucet(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM FAUCET WHERE Id = ?");
    query.addBindValue(id);
    return query.exec();
}

/**
 * @brief GgcDatabaseManager::getFaucetAmountForToday
 * @param userId
 * @return
 */
QString GgcDatabaseManager::getFaucetAmountForToday(const QString &userId)
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
 * @brief GgcDatabaseManager::getAllFaucetAmountForToday
 * @return
 */
QList<Faucet> GgcDatabaseManager::getAllFaucetAmountForToday()
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

QList<Faucet> GgcDatabaseManager::getAllFaucet()
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
