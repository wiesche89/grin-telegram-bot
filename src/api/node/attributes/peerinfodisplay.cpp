#include "peerinfodisplay.h"

/**
 * @brief PeerInfoDisplay::PeerInfoDisplay
 */
PeerInfoDisplay::PeerInfoDisplay() :
    m_height(0)
{
}

/**
 * @brief PeerInfoDisplay::capabilities
 * @return
 */
Capabilities PeerInfoDisplay::capabilities() const
{
    return m_capabilities;
}

/**
 * @brief PeerInfoDisplay::userAgent
 * @return
 */
QString PeerInfoDisplay::userAgent() const
{
    return m_userAgent;
}

/**
 * @brief PeerInfoDisplay::version
 * @return
 */
ProtocolVersion PeerInfoDisplay::version() const
{
    return m_version;
}

/**
 * @brief PeerInfoDisplay::addr
 * @return
 */
PeerAddr PeerInfoDisplay::addr() const
{
    return m_addr;
}

/**
 * @brief PeerInfoDisplay::direction
 * @return
 */
Direction PeerInfoDisplay::direction() const
{
    return m_direction;
}

/**
 * @brief PeerInfoDisplay::totalDifficulty
 * @return
 */
Difficulty PeerInfoDisplay::totalDifficulty() const
{
    return m_totalDifficulty;
}

/**
 * @brief PeerInfoDisplay::height
 * @return
 */
quint64 PeerInfoDisplay::height() const
{
    return m_height;
}

/**
 * @brief PeerInfoDisplay::setCapabilities
 * @param capabilities
 */
void PeerInfoDisplay::setCapabilities(const Capabilities &capabilities)
{
    m_capabilities = capabilities;
}

/**
 * @brief PeerInfoDisplay::setUserAgent
 * @param userAgent
 */
void PeerInfoDisplay::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
}

/**
 * @brief PeerInfoDisplay::setVersion
 * @param version
 */
void PeerInfoDisplay::setVersion(const ProtocolVersion &version)
{
    m_version = version;
}

/**
 * @brief PeerInfoDisplay::setAddr
 * @param addr
 */
void PeerInfoDisplay::setAddr(const PeerAddr &addr)
{
    m_addr = addr;
}

/**
 * @brief PeerInfoDisplay::setDirection
 * @param direction
 */
void PeerInfoDisplay::setDirection(const Direction &direction)
{
    m_direction = direction;
}

/**
 * @brief PeerInfoDisplay::setTotalDifficulty
 * @param difficulty
 */
void PeerInfoDisplay::setTotalDifficulty(const Difficulty &difficulty)
{
    m_totalDifficulty = difficulty;
}

/**
 * @brief PeerInfoDisplay::setHeight
 * @param height
 */
void PeerInfoDisplay::setHeight(quint64 height)
{
    m_height = height;
}

/**
 * @brief PeerInfoDisplay::toJson
 * @return
 */
QJsonObject PeerInfoDisplay::toJson() const
{
    QJsonObject obj;
    obj["capabilities"] = m_capabilities.toJson();
    obj["user_agent"] = m_userAgent;
    obj["version"] = m_version.toJson();
    obj["addr"] = m_addr.toJson();
    obj["direction"] = m_direction.toJson();
    obj["total_difficulty"] = m_totalDifficulty.toJson();
    obj["height"] = static_cast<double>(m_height);
    return obj;
}

/**
 * @brief PeerInfoDisplay::fromJson
 * @param obj
 * @return
 */
PeerInfoDisplay PeerInfoDisplay::fromJson(const QJsonObject &obj)
{
    PeerInfoDisplay info;
    if (obj.contains("capabilities")) {
        info.setCapabilities(Capabilities::fromJson(obj["capabilities"].toObject()));
    }
    if (obj.contains("user_agent")) {
        info.setUserAgent(obj["user_agent"].toString());
    }
    if (obj.contains("version")) {
        info.setVersion(ProtocolVersion::fromJson(obj["version"].toObject()));
    }
    if (obj.contains("addr")) {
        info.setAddr(PeerAddr::fromJson(obj["addr"].toObject()));
    }
    if (obj.contains("direction")) {
        info.setDirection(Direction::fromJson(obj["direction"].toObject()));
    }
    if (obj.contains("total_difficulty")) {
        info.setTotalDifficulty(Difficulty::fromJson(obj["total_difficulty"].toObject()));
    }
    if (obj.contains("height")) {
        info.setHeight(static_cast<quint64>(obj["height"].toDouble()));
    }
    return info;
}
