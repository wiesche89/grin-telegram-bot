#include "protocolversion.h"

/**
 * @brief ProtocolVersion::ProtocolVersion
 */
ProtocolVersion::ProtocolVersion()
{
}

/**
 * @brief ProtocolVersion::ProtocolVersion
 * @param obj
 */
ProtocolVersion::ProtocolVersion(const QJsonObject &obj) : m_obj(obj)
{
}

/**
 * @brief ProtocolVersion::toJson
 * @return
 */
QJsonObject ProtocolVersion::toJson() const
{
    return m_obj;
}

/**
 * @brief ProtocolVersion::fromJson
 * @param obj
 * @return
 */
ProtocolVersion ProtocolVersion::fromJson(const QJsonObject &obj)
{
    return ProtocolVersion(obj);
}
