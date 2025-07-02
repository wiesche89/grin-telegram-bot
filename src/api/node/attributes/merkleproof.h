#ifndef MERKLEPROOF_H
#define MERKLEPROOF_H

#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>

class MerkleProof
{
public:
    MerkleProof();

    quint64 mmrSize() const;
    void setMmrSize(quint64 size);

    QVector<QString> path() const;
    void setPath(const QVector<QString> &path);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    quint64 m_mmrSize;
    QVector<QString> m_path;
};

#endif // MERKLEPROOF_H
