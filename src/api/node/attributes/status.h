#ifndef STATUS_H
#define STATUS_H

#include <QString>
#include <QJsonObject>
#include <QJsonValue>

#include "tip.h"

class Status
{
public:
    Status();

    // Getter
    QString getChain() const;
    quint32 getProtocolVersion() const;
    QString getUserAgent() const;
    quint32 getConnections() const;
    Tip getTip() const;
    QString getSyncStatus() const;
    QJsonValue getSyncInfo() const;

    // Setter
    void setChain(const QString &chain);
    void setProtocolVersion(quint32 version);
    void setUserAgent(const QString &userAgent);
    void setConnections(quint32 connections);
    void setTip(const Tip &tip);
    void setSyncStatus(const QString &syncStatus);
    void setSyncInfo(const QJsonValue &syncInfo);

    // JSON
    static Status fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    QString m_chain;
    quint32 m_protocolVersion;
    QString m_userAgent;
    quint32 m_connections;
    Tip m_tip;
    QString m_syncStatus;
    QJsonValue m_syncInfo;
};

#endif // STATUS_H
