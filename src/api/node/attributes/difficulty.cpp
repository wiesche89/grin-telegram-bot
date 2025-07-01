#include "difficulty.h"

Difficulty::Difficulty()
{
}

Difficulty::Difficulty(const QJsonObject &obj) : m_obj(obj)
{
}

QJsonObject Difficulty::toJson() const
{
    return m_obj;
}

Difficulty Difficulty::fromJson(const QJsonObject &obj)
{
    return Difficulty(obj);
}
