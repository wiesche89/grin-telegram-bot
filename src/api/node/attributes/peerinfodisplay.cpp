#include "peerinfodisplay.h"

PeerInfoDisplay::PeerInfoDisplay() :
    m_height(0)
{
}

Capabilities PeerInfoDisplay::capabilities() const
{
    return m_capabilities;
}

QString PeerInfoDisplay::userAgent() const
{
    return m_userAgent;
}

ProtocolVersion PeerInfoDisplay::version() const
{
    return m_version;
}

PeerAddr PeerInfoDisplay::addr() const
{
    return m_addr;
}

Direction PeerInfoDisplay::direction() const
{
    return m_direction;
}

Difficulty PeerInfoDisplay::totalDifficulty() const
{
    return m_totalDifficulty;
}

quint64 PeerInfoDisplay::height() const
{
    return m_height;
}

void PeerInfoDisplay::setCapabilities(const Capabilities &capabilities)
{
    m_capabilities = capabilities;
}

void PeerInfoDisplay::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
}

void PeerInfoDisplay::setVersion(const ProtocolVersion &version)
{
    m_version = version;
}

void PeerInfoDisplay::setAddr(const PeerAddr &addr)
{
    m_addr = addr;
}

void PeerInfoDisplay::setDirection(const Direction &direction)
{
    m_direction = direction;
}

void PeerInfoDisplay::setTotalDifficulty(const Difficulty &difficulty)
{
    m_totalDifficulty = difficulty;
}

void PeerInfoDisplay::setHeight(quint64 height)
{
    m_height = height;
}

QJsonObject PeerInfoDisplay::toJson() const
{
    QJsonObject obj;
    obj["capabilities"] = m_capabilities.toJson();
    obj["user_agent"] = m_userAgent;
    obj["version"] = m_version.toJson();
    obj["addr"] = m_addr.toJson();
    obj["direction"] = m_direction.toJson();
    obj["total_difficulty"] = m_totalDifficulty.toJson();
    obj["height"] = static_cast<double>(m_height);  // Qt stores as double
    return obj;
}

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
