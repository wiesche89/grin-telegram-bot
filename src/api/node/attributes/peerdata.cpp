#include "peerdata.h"

/**
 * @brief stateToString
 * @param state
 * @return
 */
static QString stateToString(PeerData::State state)
{
    switch (state) {
    case PeerData::Healthy:
        return "Healthy";
    case PeerData::Banned:
        return "Banned";
    case PeerData::Defunct:
        return "Defunct";
    default:
        return "Unknown";
    }
}

/**
 * @brief stringToState
 * @param str
 * @return
 */
static PeerData::State stringToState(const QString &str)
{
    if (str == "Healthy") {
        return PeerData::Healthy;
    }
    if (str == "Banned") {
        return PeerData::Banned;
    }
    if (str == "Defunct") {
        return PeerData::Defunct;
    }
    return PeerData::Healthy; // Default fallback
}

/**
 * @brief reasonForBanToString
 * @param reason
 * @return
 */
static QString reasonForBanToString(PeerData::ReasonForBan reason)
{
    switch (reason) {
    case PeerData::None:
        return "None";
    case PeerData::BadBlock:
        return "BadBlock";
    case PeerData::BadCompactBlock:
        return "BadCompactBlock";
    case PeerData::BadBlockHeader:
        return "BadBlockHeader";
    case PeerData::BadTxHashSet:
        return "BadTxHashSet";
    case PeerData::ManualBan:
        return "ManualBan";
    case PeerData::FraudHeight:
        return "FraudHeight";
    case PeerData::BadHandshake:
        return "BadHandshake";
    default:
        return "Unknown";
    }
}

/**
 * @brief stringToReasonForBan
 * @param str
 * @return
 */
static PeerData::ReasonForBan stringToReasonForBan(const QString &str)
{
    if (str == "None") {
        return PeerData::None;
    }
    if (str == "BadBlock") {
        return PeerData::BadBlock;
    }
    if (str == "BadCompactBlock") {
        return PeerData::BadCompactBlock;
    }
    if (str == "BadBlockHeader") {
        return PeerData::BadBlockHeader;
    }
    if (str == "BadTxHashSet") {
        return PeerData::BadTxHashSet;
    }
    if (str == "ManualBan") {
        return PeerData::ManualBan;
    }
    if (str == "FraudHeight") {
        return PeerData::FraudHeight;
    }
    if (str == "BadHandshake") {
        return PeerData::BadHandshake;
    }
    return PeerData::None; // Default fallback
}

/**
 * @brief PeerData::PeerData
 */
PeerData::PeerData() :
    m_flags(Healthy),
    m_lastBanned(0),
    m_banReason(None),
    m_lastConnected(0)
{
}

/**
 * @brief PeerData::getAddr
 * @return
 */
PeerAddr PeerData::getAddr() const
{
    return m_addr;
}

/**
 * @brief PeerData::getCapabilities
 * @return
 */
Capabilities PeerData::getCapabilities() const
{
    return m_capabilities;
}

/**
 * @brief PeerData::getUserAgent
 * @return
 */
QString PeerData::getUserAgent() const
{
    return m_userAgent;
}

/**
 * @brief PeerData::getFlags
 * @return
 */
PeerData::State PeerData::getFlags() const
{
    return m_flags;
}

/**
 * @brief PeerData::getLastBanned
 * @return
 */
qint64 PeerData::getLastBanned() const
{
    return m_lastBanned;
}

/**
 * @brief PeerData::getBanReason
 * @return
 */
PeerData::ReasonForBan PeerData::getBanReason() const
{
    return m_banReason;
}

/**
 * @brief PeerData::getLastConnected
 * @return
 */
qint64 PeerData::getLastConnected() const
{
    return m_lastConnected;
}

/**
 * @brief PeerData::setAddr
 * @param a
 */
void PeerData::setAddr(const PeerAddr &a)
{
    m_addr = a;
}

/**
 * @brief PeerData::setCapabilities
 * @param c
 */
void PeerData::setCapabilities(const Capabilities &c)
{
    m_capabilities = c;
}

/**
 * @brief PeerData::setUserAgent
 * @param ua
 */
void PeerData::setUserAgent(const QString &ua)
{
    m_userAgent = ua;
}

/**
 * @brief PeerData::setFlags
 * @param f
 */
void PeerData::setFlags(State f)
{
    m_flags = f;
}

/**
 * @brief PeerData::setLastBanned
 * @param lb
 */
void PeerData::setLastBanned(qint64 lb)
{
    m_lastBanned = lb;
}

/**
 * @brief PeerData::setBanReason
 * @param r
 */
void PeerData::setBanReason(ReasonForBan r)
{
    m_banReason = r;
}

/**
 * @brief PeerData::setLastConnected
 * @param lc
 */
void PeerData::setLastConnected(qint64 lc)
{
    m_lastConnected = lc;
}

/**
 * @brief PeerData::fromJson
 * @param obj
 * @return
 */
PeerData PeerData::fromJson(const QJsonObject &obj)
{
    PeerData data;

    if (obj.contains("addr") && obj["addr"].isObject()) {
        data.m_addr = PeerAddr::fromJson(obj["addr"].toObject());
    }

    if (obj.contains("capabilities") && obj["capabilities"].isObject()) {
        data.m_capabilities = Capabilities::fromJson(obj["capabilities"].toObject());
    }

    if (obj.contains("user_agent")) {
        data.m_userAgent = obj["user_agent"].toString();
    }

    if (obj.contains("flags")) {
        data.m_flags = stringToState(obj["flags"].toString());
    }

    if (obj.contains("last_banned")) {
        data.m_lastBanned = obj["last_banned"].toVariant().toLongLong();
    }

    if (obj.contains("ban_reason")) {
        data.m_banReason = stringToReasonForBan(obj["ban_reason"].toString());
    }

    if (obj.contains("last_connected")) {
        data.m_lastConnected = obj["last_connected"].toVariant().toLongLong();
    }

    return data;
}

/**
 * @brief PeerData::toJson
 * @return
 */
QJsonObject PeerData::toJson() const
{
    QJsonObject obj;
    obj["addr"] = m_addr.toJson();
    obj["capabilities"] = m_capabilities.toJson();
    obj["user_agent"] = m_userAgent;
    obj["flags"] = stateToString(m_flags);
    obj["last_banned"] = QString::number(m_lastBanned);
    obj["ban_reason"] = reasonForBanToString(m_banReason);
    obj["last_connected"] = QString::number(m_lastConnected);
    return obj;
}
