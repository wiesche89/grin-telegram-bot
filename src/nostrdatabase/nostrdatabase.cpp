#include "nostrdatabase/nostrdatabase.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>
#include <QtGlobal>
#include <QDir>
#include <QFileInfo>

NostrDatabase::NostrDatabase(const QString &dbPath, QObject *parent) :
    QObject(parent)
{
    m_connectionName = "nostr_" + QString::number(reinterpret_cast<quintptr>(this));
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);
}

NostrDatabase::~NostrDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool NostrDatabase::initialize()
{
    QFileInfo fileInfo(m_db.databaseName());
    QDir databaseDir = fileInfo.dir();
    if (!databaseDir.exists() && !databaseDir.mkpath(databaseDir.path())) {
        qWarning() << "Failed to create nostr database directory:" << databaseDir.path();
        return false;
    }

    if (!m_db.open()) {
        qWarning() << "Failed to open nostr database:" << m_db.lastError().text();
        return false;
    }

    return ensureTables();
}

bool NostrDatabase::ensureTables()
{
    QSqlQuery query(m_db);
    static const char *createTable = R"(
        CREATE TABLE IF NOT EXISTS nostr_events (
            event_id TEXT PRIMARY KEY,
            pubkey TEXT,
            kind INTEGER,
            content TEXT,
            relay TEXT,
            received_at INTEGER NOT NULL,
            acknowledged INTEGER NOT NULL DEFAULT 0,
            acknowledged_at INTEGER
        )
    )";

    if (!query.exec(createTable)) {
        qWarning() << "Failed to create nostr_events table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool NostrDatabase::eventExists(const QString &eventId) const
{
    if (eventId.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM nostr_events WHERE event_id = ? LIMIT 1");
    query.addBindValue(eventId);

    if (!query.exec()) {
        qWarning() << "Failed to check nostr event existence:" << query.lastError().text();
        return false;
    }

    return query.next();
}

bool NostrDatabase::recordEvent(const QString &eventId,
                                const QString &pubkey,
                                int kind,
                                const QString &content,
                                const QString &relay,
                                qint64 receivedAt)
{
    if (eventId.isEmpty()) {
        return false;
    }

    qint64 storedAt = receivedAt > 0 ? receivedAt : QDateTime::currentSecsSinceEpoch();

    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO nostr_events (event_id, pubkey, kind, content, relay, received_at) VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(eventId);
    query.addBindValue(pubkey);
    query.addBindValue(kind);
    query.addBindValue(content);
    query.addBindValue(relay);
    query.addBindValue(storedAt);

    if (!query.exec()) {
        qWarning() << "Failed to record nostr event" << eventId << ":" << query.lastError().text();
        return false;
    }

    return true;
}

bool NostrDatabase::acknowledgeEvent(const QString &eventId)
{
    if (eventId.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE nostr_events SET acknowledged = 1, acknowledged_at = ? WHERE event_id = ?");
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(eventId);

    if (!query.exec()) {
        qWarning() << "Failed to acknowledge nostr event" << eventId << ":" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}
