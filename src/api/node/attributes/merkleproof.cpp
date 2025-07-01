#include "merkleproof.h"

MerkleProof::MerkleProof() : m_mmrSize(0)
{
}

quint64 MerkleProof::mmrSize() const
{
    return m_mmrSize;
}

void MerkleProof::setMmrSize(quint64 size)
{
    m_mmrSize = size;
}

QVector<QString> MerkleProof::path() const
{
    return m_path;
}

void MerkleProof::setPath(const QVector<QString> &path)
{
    m_path = path;
}

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
