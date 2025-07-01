#include "capabilities.h"

Capabilities::Capabilities()
{
}

Capabilities::Capabilities(const QJsonObject &obj) : m_obj(obj)
{
}

QJsonObject Capabilities::toJson() const
{
    return m_obj;
}

Capabilities Capabilities::fromJson(const QJsonObject &obj)
{
    return Capabilities(obj);
}
