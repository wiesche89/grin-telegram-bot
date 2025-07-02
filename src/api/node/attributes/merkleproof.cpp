#include "merkleproof.h"

/**
 * @brief MerkleProof::MerkleProof
 */
MerkleProof::MerkleProof() :
    m_mmrSize(0)
{
}

/**
 * @brief MerkleProof::mmrSize
 * @return
 */
quint64 MerkleProof::mmrSize() const
{
    return m_mmrSize;
}

/**
 * @brief MerkleProof::setMmrSize
 * @param size
 */
void MerkleProof::setMmrSize(quint64 size)
{
    m_mmrSize = size;
}

/**
 * @brief MerkleProof::path
 * @return
 */
QVector<QString> MerkleProof::path() const
{
    return m_path;
}

/**
 * @brief MerkleProof::setPath
 * @param path
 */
void MerkleProof::setPath(const QVector<QString> &path)
{
    m_path = path;
}

/**
 * @brief MerkleProof::fromJson
 * @param json
 */
void MerkleProof::fromJson(const QJsonObject &json)
{
    m_mmrSize = json.value("mmr_size").toVariant().toULongLong();
    m_path.clear();
    if (json.contains("path") && json["path"].isArray()) {
        QJsonArray arr = json["path"].toArray();
        for (const QJsonValue &val : arr) {
            m_path.append(val.toString());
        }
    }
}

/**
 * @brief MerkleProof::toJson
 * @return
 */
QJsonObject MerkleProof::toJson() const
{
    QJsonObject json;
    json["mmr_size"] = QString::number(m_mmrSize);

    QJsonArray arr;
    for (const QString &hash : m_path) {
        arr.append(hash);
    }
    json["path"] = arr;

    return json;
}
