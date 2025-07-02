#include "difficulty.h"

/**
 * @brief Difficulty::Difficulty
 */
Difficulty::Difficulty()
{
}

/**
 * @brief Difficulty::Difficulty
 * @param obj
 */
Difficulty::Difficulty(const QJsonObject &obj) : m_obj(obj)
{
}

/**
 * @brief Difficulty::toJson
 * @return
 */
QJsonObject Difficulty::toJson() const
{
    return m_obj;
}

/**
 * @brief Difficulty::fromJson
 * @param obj
 * @return
 */
Difficulty Difficulty::fromJson(const QJsonObject &obj)
{
    return Difficulty(obj);
}
