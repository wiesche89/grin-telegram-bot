#ifndef NOSTRDATABASE_H
#define NOSTRDATABASE_H

#include <QObject>
#include <QtGlobal>
#include <QSqlDatabase>
#include <QString>

class NostrDatabase : public QObject
{
    Q_OBJECT

public:
    explicit NostrDatabase(const QString &dbPath, QObject *parent = nullptr);
    ~NostrDatabase();

    bool initialize();
    bool eventExists(const QString &eventId) const;
    bool recordEvent(const QString &eventId,
                     const QString &pubkey,
                     int kind,
                     const QString &content,
                     const QString &relay,
                     qint64 receivedAt = -1);
    bool acknowledgeEvent(const QString &eventId);

private:
    bool ensureTables();

    QSqlDatabase m_db;
    QString m_connectionName;
};

#endif // NOSTRDATABASE_H
