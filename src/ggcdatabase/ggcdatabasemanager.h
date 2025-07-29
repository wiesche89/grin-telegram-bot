#ifndef GGCDATABASEMANAGER_H
#define GGCDATABASEMANAGER_H

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

class GgcDatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit GgcDatabaseManager(QObject *parent = nullptr);
    ~GgcDatabaseManager();

    bool connectToDatabase(const QString &dbPath);
    void closeDatabase();
    QSqlDatabase getDatabase() const;

    // Donate CRUD
    bool insertDonate(const Donate &donate);
    Donate getDonateById(int id);
    bool updateDonate(const Donate &donate);
    bool deleteDonate(int id);
    QList<Donate> getAllDonate();

    // Faucet CRUD
    bool insertFaucet(const Faucet &faucet);
    Faucet getFaucetById(int id);
    bool updateFaucet(const Faucet &faucet);
    bool deleteFaucet(int id);
    QString getFaucetAmountForToday(const QString &userId);
    QList<Faucet> getAllFaucetAmountForToday();
    QList<Faucet> getAllFaucet();

private:
    QSqlDatabase db;
};

#endif // GGCDATABASEMANAGER_H
