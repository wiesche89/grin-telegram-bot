#include "capabilities.h"

/**
 * @brief Capabilities::Capabilities
 */
Capabilities::Capabilities()
{
}

/**
 * @brief Capabilities::Capabilities
 * @param obj
 */
Capabilities::Capabilities(const QJsonObject &obj) : m_obj(obj)
{
}

/**
 * @brief Capabilities::toJson
 * @return
 */
QJsonObject Capabilities::toJson() const
{
    return m_obj;
}

/**
 * @brief Capabilities::fromJson
 * @param obj
 * @return
 */
Capabilities Capabilities::fromJson(const QJsonObject &obj)
{
    return Capabilities(obj);
}
