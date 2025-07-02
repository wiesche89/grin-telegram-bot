#include "status.h"

/**
 * @brief Status::Status
 */
Status::Status() :
    m_protocolVersion(0),
    m_connections(0),
    m_syncInfo(QJsonValue::Null)
{
}

/**
 * @brief Status::getChain
 * @return
 */
QString Status::getChain() const
{
    return m_chain;
}

/**
 * @brief Status::getProtocolVersion
 * @return
 */
quint32 Status::getProtocolVersion() const
{
    return m_protocolVersion;
}

/**
 * @brief Status::getUserAgent
 * @return
 */
QString Status::getUserAgent() const
{
    return m_userAgent;
}

/**
 * @brief Status::getConnections
 * @return
 */
quint32 Status::getConnections() const
{
    return m_connections;
}

/**
 * @brief Status::getTip
 * @return
 */
Tip Status::getTip() const
{
    return m_tip;
}

/**
 * @brief Status::getSyncStatus
 * @return
 */
QString Status::getSyncStatus() const
{
    return m_syncStatus;
}

/**
 * @brief Status::getSyncInfo
 * @return
 */
QJsonValue Status::getSyncInfo() const
{
    return m_syncInfo;
}

/**
 * @brief Status::setChain
 * @param c
 */
void Status::setChain(const QString &c)
{
    m_chain = c;
}

/**
 * @brief Status::setProtocolVersion
 * @param v
 */
void Status::setProtocolVersion(quint32 v)
{
    m_protocolVersion = v;
}

/**
 * @brief Status::setUserAgent
 * @param ua
 */
void Status::setUserAgent(const QString &ua)
{
    m_userAgent = ua;
}

/**
 * @brief Status::setConnections
 * @param c
 */
void Status::setConnections(quint32 c)
{
    m_connections = c;
}

/**
 * @brief Status::setTip
 * @param t
 */
void Status::setTip(const Tip &t)
{
    m_tip = t;
}

/**
 * @brief Status::setSyncStatus
 * @param s
 */
void Status::setSyncStatus(const QString &s)
{
    m_syncStatus = s;
}

/**
 * @brief Status::setSyncInfo
 * @param val
 */
void Status::setSyncInfo(const QJsonValue &val)
{
    m_syncInfo = val;
}

/**
 * @brief Status::fromJson
 * @param obj
 * @return
 */
Status Status::fromJson(const QJsonObject &obj)
{
    Status status;
    status.m_chain = obj.value("chain").toString();
    status.m_protocolVersion = static_cast<quint32>(obj.value("protocol_version").toInt());
    status.m_userAgent = obj.value("user_agent").toString();
    status.m_connections = static_cast<quint32>(obj.value("connections").toInt());

    // tip muss ein Objekt sein
    if (obj.contains("tip") && obj.value("tip").isObject()) {
        status.m_tip = Tip::fromJson(obj.value("tip").toObject());
    }

    status.m_syncStatus = obj.value("sync_status").toString();

    if (obj.contains("sync_info")) {
        status.m_syncInfo = obj.value("sync_info");
    } else {
        status.m_syncInfo = QJsonValue::Null;
    }

    return status;
}

/**
 * @brief Status::toJson
 * @return
 */
QJsonObject Status::toJson() const
{
    QJsonObject obj;
    obj.insert("chain", m_chain);
    obj.insert("protocol_version", static_cast<int>(m_protocolVersion));
    obj.insert("user_agent", m_userAgent);
    obj.insert("connections", static_cast<int>(m_connections));
    obj.insert("tip", m_tip.toJson());
    obj.insert("sync_status", m_syncStatus);
    obj.insert("sync_info", m_syncInfo);
    return obj;
}
