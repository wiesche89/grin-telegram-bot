#include "status.h"

Status::Status() :
    protocolVersion(0),
    connections(0),
    syncInfo(QJsonValue::Null)
{
}

// Getter
QString Status::getChain() const
{
    return chain;
}

quint32 Status::getProtocolVersion() const
{
    return protocolVersion;
}

QString Status::getUserAgent() const
{
    return userAgent;
}

quint32 Status::getConnections() const
{
    return connections;
}

Tip Status::getTip() const
{
    return tip;
}

QString Status::getSyncStatus() const
{
    return syncStatus;
}

QJsonValue Status::getSyncInfo() const
{
    return syncInfo;
}

// Setter
void Status::setChain(const QString &c)
{
    chain = c;
}

void Status::setProtocolVersion(quint32 v)
{
    protocolVersion = v;
}

void Status::setUserAgent(const QString &ua)
{
    userAgent = ua;
}

void Status::setConnections(quint32 c)
{
    connections = c;
}

void Status::setTip(const Tip &t)
{
    tip = t;
}

void Status::setSyncStatus(const QString &s)
{
    syncStatus = s;
}

void Status::setSyncInfo(const QJsonValue &val)
{
    syncInfo = val;
}

// JSON parsing
Status Status::fromJson(const QJsonObject &obj)
{
    Status status;
    status.chain = obj.value("chain").toString();
    status.protocolVersion = static_cast<quint32>(obj.value("protocol_version").toInt());
    status.userAgent = obj.value("user_agent").toString();
    status.connections = static_cast<quint32>(obj.value("connections").toInt());

    // tip muss ein Objekt sein
    if (obj.contains("tip") && obj.value("tip").isObject()) {
        status.tip = Tip::fromJson(obj.value("tip").toObject());
    }

    status.syncStatus = obj.value("sync_status").toString();

    if (obj.contains("sync_info")) {
        status.syncInfo = obj.value("sync_info");
    } else {
        status.syncInfo = QJsonValue::Null;
    }

    return status;
}

QJsonObject Status::toJson() const
{
    QJsonObject obj;
    obj.insert("chain", chain);
    obj.insert("protocol_version", static_cast<int>(protocolVersion));
    obj.insert("user_agent", userAgent);
    obj.insert("connections", static_cast<int>(connections));
    obj.insert("tip", tip.toJson());
    obj.insert("sync_status", syncStatus);
    obj.insert("sync_info", syncInfo);
    return obj;
}
