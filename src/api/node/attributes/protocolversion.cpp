#include "protocolversion.h"

ProtocolVersion::ProtocolVersion()
{
}

ProtocolVersion::ProtocolVersion(const QJsonObject &obj) : m_obj(obj)
{
}

QJsonObject ProtocolVersion::toJson() const
{
    return m_obj;
}

ProtocolVersion ProtocolVersion::fromJson(const QJsonObject &obj)
{
    return ProtocolVersion(obj);
}
