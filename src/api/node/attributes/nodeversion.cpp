#include "nodeversion.h"

/**
 * @brief NodeVersion::NodeVersion
 * @param nodeVersion
 * @param blockHeaderVersion
 */
NodeVersion::NodeVersion(const QString &nodeVersion, quint64 blockHeaderVersion) :
    m_nodeVersion(nodeVersion),
    m_blockHeaderVersion(blockHeaderVersion)
{
}

/**
 * @brief NodeVersion::nodeVersion
 * @return
 */
QString NodeVersion::nodeVersion() const
{
    return m_nodeVersion;
}

/**
 * @brief NodeVersion::blockHeaderVersion
 * @return
 */
quint64 NodeVersion::blockHeaderVersion() const
{
    return m_blockHeaderVersion;
}

/**
 * @brief NodeVersion::setNodeVersion
 * @param nodeVersion
 */
void NodeVersion::setNodeVersion(const QString &nodeVersion)
{
    m_nodeVersion = nodeVersion;
}

/**
 * @brief NodeVersion::setBlockHeaderVersion
 * @param blockHeaderVersion
 */
void NodeVersion::setBlockHeaderVersion(quint64 blockHeaderVersion)
{
    m_blockHeaderVersion = blockHeaderVersion;
}

/**
 * @brief NodeVersion::fromJson
 * @param obj
 * @return
 */
NodeVersion NodeVersion::fromJson(const QJsonObject &obj)
{
    NodeVersion v;
    if (obj.contains("node_version") && obj["node_version"].isString()) {
        v.setNodeVersion(obj["node_version"].toString());
    }
    if (obj.contains("block_header_version") && obj["block_header_version"].isDouble()) {
        v.setBlockHeaderVersion(static_cast<uint16_t>(obj["block_header_version"].toInt()));
    }
    return v;
}

/**
 * @brief NodeVersion::toJson
 * @return
 */
QJsonObject NodeVersion::toJson() const
{
    QJsonObject obj;
    obj["node_version"] = m_nodeVersion;
    obj["block_header_version"] = static_cast<int>(m_blockHeaderVersion);
    return obj;
}
