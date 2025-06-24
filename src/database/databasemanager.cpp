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
bool DatabaseManager::insertDonate(const Donate& donate) {
    QSqlQuery query;
    query.prepare("INSERT INTO DONATE (UserId, Username, Amount, Date) VALUES (?, ?, ?, ?)");
    query.addBindValue(donate.userId());
    query.addBindValue(donate.username());
    query.addBindValue(donate.amount());
    query.addBindValue(donate.date());
    return query.exec();
}

Donate* DatabaseManager::getDonateById(int id) {
    QSqlQuery query;
    query.prepare("SELECT Id, UserId, Username, Amount, Date FROM DONATE WHERE Id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        return new Donate(query.value(0).toInt(),
                          query.value(1).toString(),
                          query.value(2).toString(),
                          query.value(3).toString(),
                          query.value(4).toString());
    }
    return nullptr;
}

bool DatabaseManager::updateDonate(const Donate& donate) {
    QSqlQuery query;
    query.prepare("UPDATE DONATE SET UserId = ?, Username = ?, Amount = ?, Date = ? WHERE Id = ?");
    query.addBindValue(donate.userId());
    query.addBindValue(donate.username());
    query.addBindValue(donate.amount());
    query.addBindValue(donate.date());
    query.addBindValue(donate.id());
    return query.exec();
}

bool DatabaseManager::deleteDonate(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM DONATE WHERE Id = ?");
    query.addBindValue(id);
    return query.exec();
}

// ------------------ Faucet CRUD ------------------
bool DatabaseManager::insertFaucet(const Faucet& faucet) {
    QSqlQuery query;
    query.prepare("INSERT INTO FAUCET (UserId, Username, Amount, Date) VALUES (?, ?, ?, ?)");
    query.addBindValue(faucet.userId());
    query.addBindValue(faucet.username());
    query.addBindValue(faucet.amount());
    query.addBindValue(faucet.date());
    return query.exec();
}

Faucet* DatabaseManager::getFaucetById(int id) {
    QSqlQuery query;
    query.prepare("SELECT Id, UserId, Username, Amount, Date FROM FAUCET WHERE Id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        return new Faucet(query.value(0).toInt(),
                          query.value(1).toString(),
                          query.value(2).toString(),
                          query.value(3).toString(),
                          query.value(4).toString());
    }
    return nullptr;
}

bool DatabaseManager::updateFaucet(const Faucet& faucet) {
    QSqlQuery query;
    query.prepare("UPDATE FAUCET SET UserId = ?, Username = ?, Amount = ?, Date = ? WHERE Id = ?");
    query.addBindValue(faucet.userId());
    query.addBindValue(faucet.username());
    query.addBindValue(faucet.amount());
    query.addBindValue(faucet.date());
    query.addBindValue(faucet.id());
    return query.exec();
}

bool DatabaseManager::deleteFaucet(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM FAUCET WHERE Id = ?");
    query.addBindValue(id);
    return query.exec();
}

QString DatabaseManager::getFaucetAmountForToday(const QString& userId) {
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
        return QString::number(total, 'f',0);


        return query.value(0).toString();
    }

    return "0";
}
