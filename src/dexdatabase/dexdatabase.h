#ifndef DEXDATABASE_H
#define DEXDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

class DexDatabase : public QObject
{
    Q_OBJECT
public:
    explicit DexDatabase(const QString &path, QObject *parent = nullptr);
    bool initialize();

    // Abfragen
    bool ensureTables();
    QSqlDatabase database() const;

private:
    QSqlDatabase m_db;
    QString m_connectionName;
};

#endif // DEXDATABASE_H
