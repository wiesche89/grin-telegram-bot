#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QObject>
#include <QList>
#include <QSqlQuery>
#include <QDateTime>


#include "faucet.h"
#include "donate.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool connectToDatabase(const QString &dbPath);
    void closeDatabase();
    QSqlDatabase getDatabase() const;

    // Donate CRUD
    bool insertDonate(const Donate& donate);
    Donate* getDonateById(int id);
    bool updateDonate(const Donate& donate);
    bool deleteDonate(int id);

    // Faucet CRUD
    bool insertFaucet(const Faucet& faucet);
    Faucet getFaucetById(int id);
    bool updateFaucet(const Faucet& faucet);
    bool deleteFaucet(int id);
    QString getFaucetAmountForToday(const QString& userId);

private:
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
