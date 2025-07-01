#ifndef OUTPUTPRINTABLE_H
#define OUTPUTPRINTABLE_H

#include <QString>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include "commitment.h"
#include "merkleproof.h"

enum class OutputType {
    OutputTypeCoinbase,
    OutputTypeTransaction,
    OutputTypeUnknown = 256,
};

class OutputPrintable
{
public:
    OutputPrintable();

    OutputType outputType() const;
    void setOutputType(OutputType type);

    Commitment commit() const;
    void setCommit(const Commitment &commit);

    bool spent() const;
    void setSpent(bool spent);

    QString proof() const;
    void setProof(const QString &proof);

    QString proofHash() const;
    void setProofHash(const QString &hash);

    QVariant blockHeight() const;  // optional: can be invalid QVariant if no value
    void setBlockHeight(const QVariant &height);

    MerkleProof merkleProof() const;
    void setMerkleProof(const MerkleProof &proof);

    quint64 mmrIndex() const;
    void setMmrIndex(quint64 index);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    OutputType m_outputType;
    Commitment m_commit;
    bool m_spent;
    QString m_proof;          // optional, empty if none
    QString m_proofHash;
    QVariant m_blockHeight;   // optional u64, use QVariant to allow invalid
    MerkleProof m_merkleProof; // optional, default empty
    quint64 m_mmrIndex;
};

#endif // OUTPUTPRINTABLE_H
