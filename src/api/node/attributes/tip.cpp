#include "tip.h"

/**
 * @brief Tip::Tip
 */
Tip::Tip() :
    m_height(0),
    m_totalDifficulty(0)
{
}

/**
 * @brief Tip::Tip
 * @param height
 * @param lastBlockPushed
 * @param prevBlockToLast
 * @param totalDifficulty
 */
Tip::Tip(quint64 height, const QString &lastBlockPushed, const QString &prevBlockToLast, quint64 totalDifficulty) :
    m_height(height),
    m_lastBlockPushed(lastBlockPushed),
    m_prevBlockToLast(prevBlockToLast),
    m_totalDifficulty(totalDifficulty)
{
}

/**
 * @brief Tip::height
 * @return
 */
quint64 Tip::height() const
{
    return m_height;
}

/**
 * @brief Tip::lastBlockPushed
 * @return
 */
QString Tip::lastBlockPushed() const
{
    return m_lastBlockPushed;
}

/**
 * @brief Tip::prevBlockToLast
 * @return
 */
QString Tip::prevBlockToLast() const
{
    return m_prevBlockToLast;
}

/**
 * @brief Tip::totalDifficulty
 * @return
 */
quint64 Tip::totalDifficulty() const
{
    return m_totalDifficulty;
}

/**
 * @brief Tip::setHeight
 * @param height
 */
void Tip::setHeight(quint64 height)
{
    m_height = height;
}

/**
 * @brief Tip::setLastBlockPushed
 * @param lastBlockPushed
 */
void Tip::setLastBlockPushed(const QString &lastBlockPushed)
{
    m_lastBlockPushed = lastBlockPushed;
}

/**
 * @brief Tip::setPrevBlockToLast
 * @param prevBlockToLast
 */
void Tip::setPrevBlockToLast(const QString &prevBlockToLast)
{
    m_prevBlockToLast = prevBlockToLast;
}

/**
 * @brief Tip::setTotalDifficulty
 * @param totalDifficulty
 */
void Tip::setTotalDifficulty(quint64 totalDifficulty)
{
    m_totalDifficulty = totalDifficulty;
}

/**
 * @brief Tip::fromJson
 * @param json
 * @return
 */
Tip Tip::fromJson(const QJsonObject &json)
{
    Tip tip;
    if (json.contains("height") && json["height"].isDouble()) {
        tip.setHeight(static_cast<quint64>(json["height"].toDouble()));
    }
    if (json.contains("last_block_pushed") && json["last_block_pushed"].isString()) {
        tip.setLastBlockPushed(json["last_block_pushed"].toString());
    }
    if (json.contains("prev_block_to_last") && json["prev_block_to_last"].isString()) {
        tip.setPrevBlockToLast(json["prev_block_to_last"].toString());
    }
    if (json.contains("total_difficulty") && json["total_difficulty"].isDouble()) {
        tip.setTotalDifficulty(static_cast<quint64>(json["total_difficulty"].toDouble()));
    }
    return tip;
}

/**
 * @brief Tip::toJson
 * @return
 */
QJsonObject Tip::toJson() const
{
    QJsonObject json;
    json["height"] = static_cast<double>(m_height);
    json["last_block_pushed"] = m_lastBlockPushed;
    json["prev_block_to_last"] = m_prevBlockToLast;
    json["total_difficulty"] = static_cast<double>(m_totalDifficulty);
    return json;
}
