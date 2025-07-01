#include "nodeversion.h"

// Constructor
NodeVersion::NodeVersion(const QString &nodeVersion, uint16_t blockHeaderVersion) :
    m_nodeVersion(nodeVersion),
    m_blockHeaderVersion(blockHeaderVersion)
{
}

// Getter implementations
QString NodeVersion::nodeVersion() const
{
    return m_nodeVersion;
}

uint16_t NodeVersion::blockHeaderVersion() const
{
    return m_blockHeaderVersion;
}

// Setter implementations
void NodeVersion::setNodeVersion(const QString &nodeVersion)
{
    m_nodeVersion = nodeVersion;
}

void NodeVersion::setBlockHeaderVersion(uint16_t blockHeaderVersion)
{
    m_blockHeaderVersion = blockHeaderVersion;
}

// JSON deserialization
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

// JSON serialization
QJsonObject NodeVersion::toJson() const
{
    QJsonObject obj;
    obj["node_version"] = m_nodeVersion;
    obj["block_header_version"] = static_cast<int>(m_blockHeaderVersion);
    return obj;
}
