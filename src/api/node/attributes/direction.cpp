#include "direction.h"

/**
 * @brief Direction::Direction
 */
Direction::Direction()
{
}

/**
 * @brief Direction::Direction
 * @param obj
 */
Direction::Direction(const QJsonObject &obj) : m_obj(obj)
{
}

/**
 * @brief Direction::toJson
 * @return
 */
QJsonObject Direction::toJson() const
{
    return m_obj;
}

/**
 * @brief Direction::fromJson
 * @param obj
 * @return
 */
Direction Direction::fromJson(const QJsonObject &obj)
{
    return Direction(obj);
}
