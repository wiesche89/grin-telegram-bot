#ifndef TIPPINGDATABASE_H
#define TIPPINGDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QString>

class TippingDatabase : public QObject
{
    Q_OBJECT

public:
    explicit TippingDatabase(const QString &dbPath, QObject *parent = nullptr);
    ~TippingDatabase();

    bool initialize();
    bool recordTransaction(const QString &fromUser, const QString &toUser, int amount, const QString &type, const QString &reference = QString());
    int getBalance(const QString &userId);
    bool updateBalance(const QString &userId, int amountDelta);
    bool setBalance(const QString &userId, int balance);

private:
    QSqlDatabase m_db;
    QSqlQuery *m_query = nullptr;
    QString m_connectionName;

    bool ensureTables();
};

#endif // TIPPINGDATABASE_H
