#include "direction.h"

Direction::Direction()
{
}

Direction::Direction(const QJsonObject &obj) : m_obj(obj)
{
}

QJsonObject Direction::toJson() const
{
    return m_obj;
}

Direction Direction::fromJson(const QJsonObject &obj)
{
    return Direction(obj);
}
