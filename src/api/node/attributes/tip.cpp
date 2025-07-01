#include "tip.h"

Tip::Tip() :
    m_height(0),
    m_totalDifficulty(0)
{
}

Tip::Tip(uint64_t height, const QString &lastBlockPushed, const QString &prevBlockToLast, uint64_t totalDifficulty) :
    m_height(height),
    m_lastBlockPushed(lastBlockPushed),
    m_prevBlockToLast(prevBlockToLast),
    m_totalDifficulty(totalDifficulty)
{
}

uint64_t Tip::height() const
{
    return m_height;
}

QString Tip::lastBlockPushed() const
{
    return m_lastBlockPushed;
}

QString Tip::prevBlockToLast() const
{
    return m_prevBlockToLast;
}

uint64_t Tip::totalDifficulty() const
{
    return m_totalDifficulty;
}

void Tip::setHeight(uint64_t height)
{
    m_height = height;
}

void Tip::setLastBlockPushed(const QString &lastBlockPushed)
{
    m_lastBlockPushed = lastBlockPushed;
}

void Tip::setPrevBlockToLast(const QString &prevBlockToLast)
{
    m_prevBlockToLast = prevBlockToLast;
}

void Tip::setTotalDifficulty(uint64_t totalDifficulty)
{
    m_totalDifficulty = totalDifficulty;
}

Tip Tip::fromJson(const QJsonObject &json)
{
    Tip tip;
    if (json.contains("height") && json["height"].isDouble()) {
        tip.setHeight(static_cast<uint64_t>(json["height"].toDouble()));
    }
    if (json.contains("last_block_pushed") && json["last_block_pushed"].isString()) {
        tip.setLastBlockPushed(json["last_block_pushed"].toString());
    }
    if (json.contains("prev_block_to_last") && json["prev_block_to_last"].isString()) {
        tip.setPrevBlockToLast(json["prev_block_to_last"].toString());
    }
    if (json.contains("total_difficulty") && json["total_difficulty"].isDouble()) {
        tip.setTotalDifficulty(static_cast<uint64_t>(json["total_difficulty"].toDouble()));
    }
    return tip;
}

QJsonObject Tip::toJson() const
{
    QJsonObject json;
    json["height"] = static_cast<double>(m_height);
    json["last_block_pushed"] = m_lastBlockPushed;
    json["prev_block_to_last"] = m_prevBlockToLast;
    json["total_difficulty"] = static_cast<double>(m_totalDifficulty);
    return json;
}
