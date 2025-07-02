#include "blocklisting.h"

/**
 * @brief BlockListing::BlockListing
 */
BlockListing::BlockListing() :
    m_lastRetrievedHeight(0)
{
}

/**
 * @brief BlockListing::lastRetrievedHeight
 * @return
 */
quint64 BlockListing::lastRetrievedHeight() const
{
    return m_lastRetrievedHeight;
}

/**
 * @brief BlockListing::blocks
 * @return
 */
QVector<BlockPrintable> BlockListing::blocks() const
{
    return m_blocks;
}

/**
 * @brief BlockListing::setLastRetrievedHeight
 * @param height
 */
void BlockListing::setLastRetrievedHeight(quint64 height)
{
    m_lastRetrievedHeight = height;
}

/**
 * @brief BlockListing::setBlocks
 * @param blocks
 */
void BlockListing::setBlocks(const QVector<BlockPrintable> &blocks)
{
    m_blocks = blocks;
}

/**
 * @brief BlockListing::fromJson
 * @param json
 */
void BlockListing::fromJson(const QJsonObject &json)
{
    if (json.contains("last_retrieved_height") && json["last_retrieved_height"].isDouble()) {
        m_lastRetrievedHeight = static_cast<quint64>(json["last_retrieved_height"].toDouble());
    }

    m_blocks.clear();
    if (json.contains("blocks") && json["blocks"].isArray()) {
        QJsonArray arr = json["blocks"].toArray();
        for (const auto &v : arr) {
            if (v.isObject()) {
                BlockPrintable block;
                block.fromJson(v.toObject());
                m_blocks.append(block);
            }
        }
    }
}

/**
 * @brief BlockListing::toJson
 * @return
 */
QJsonObject BlockListing::toJson() const
{
    QJsonObject json;
    json["last_retrieved_height"] = static_cast<double>(m_lastRetrievedHeight);

    QJsonArray blocksArray;
    for (const auto &block : m_blocks) {
        blocksArray.append(block.toJson());
    }

    json["blocks"] = blocksArray;
    return json;
}
