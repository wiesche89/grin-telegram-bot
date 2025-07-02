#include "outputprintable.h"

/**
 * @brief OutputPrintable::OutputPrintable
 */
OutputPrintable::OutputPrintable() :
    m_outputType(OutputType::OutputTypeUnknown),
    m_spent(false),
    m_blockHeight(QVariant()),
    m_mmrIndex(0)
{
}

/**
 * @brief OutputPrintable::outputType
 * @return
 */
OutputType OutputPrintable::outputType() const
{
    return m_outputType;
}

/**
 * @brief OutputPrintable::setOutputType
 * @param type
 */
void OutputPrintable::setOutputType(OutputType type)
{
    m_outputType = type;
}

/**
 * @brief OutputPrintable::commit
 * @return
 */
Commitment OutputPrintable::commit() const
{
    return m_commit;
}

/**
 * @brief OutputPrintable::setCommit
 * @param commit
 */
void OutputPrintable::setCommit(const Commitment &commit)
{
    m_commit = commit;
}

/**
 * @brief OutputPrintable::spent
 * @return
 */
bool OutputPrintable::spent() const
{
    return m_spent;
}

/**
 * @brief OutputPrintable::setSpent
 * @param spent
 */
void OutputPrintable::setSpent(bool spent)
{
    m_spent = spent;
}

/**
 * @brief OutputPrintable::proof
 * @return
 */
QString OutputPrintable::proof() const
{
    return m_proof;
}

/**
 * @brief OutputPrintable::setProof
 * @param proof
 */
void OutputPrintable::setProof(const QString &proof)
{
    m_proof = proof;
}

/**
 * @brief OutputPrintable::proofHash
 * @return
 */
QString OutputPrintable::proofHash() const
{
    return m_proofHash;
}

/**
 * @brief OutputPrintable::setProofHash
 * @param hash
 */
void OutputPrintable::setProofHash(const QString &hash)
{
    m_proofHash = hash;
}

/**
 * @brief OutputPrintable::blockHeight
 * @return
 */
QVariant OutputPrintable::blockHeight() const
{
    return m_blockHeight;
}

/**
 * @brief OutputPrintable::setBlockHeight
 * @param height
 */
void OutputPrintable::setBlockHeight(const QVariant &height)
{
    m_blockHeight = height;
}

/**
 * @brief OutputPrintable::merkleProof
 * @return
 */
MerkleProof OutputPrintable::merkleProof() const
{
    return m_merkleProof;
}

/**
 * @brief OutputPrintable::setMerkleProof
 * @param proof
 */
void OutputPrintable::setMerkleProof(const MerkleProof &proof)
{
    m_merkleProof = proof;
}

/**
 * @brief OutputPrintable::mmrIndex
 * @return
 */
quint64 OutputPrintable::mmrIndex() const
{
    return m_mmrIndex;
}

/**
 * @brief OutputPrintable::setMmrIndex
 * @param index
 */
void OutputPrintable::setMmrIndex(quint64 index)
{
    m_mmrIndex = index;
}

/**
 * @brief OutputPrintable::fromJson
 * @param json
 */
void OutputPrintable::fromJson(const QJsonObject &json)
{
    QString typeStr = json.value("output_type").toString();
    if (typeStr == "Coinbase") {
        m_outputType = OutputType::OutputTypeCoinbase;
    } else if (typeStr == "Transaction") {
        m_outputType = OutputType::OutputTypeTransaction;
    } else {
        m_outputType = OutputType::OutputTypeUnknown;
    }

    m_commit.fromJson(json.value("commit").toObject());
    m_spent = json.value("spent").toBool();

    if (json.contains("proof") && !json.value("proof").isNull()) {
        m_proof = json.value("proof").toString();
    } else {
        m_proof.clear();
    }

    m_proofHash = json.value("proof_hash").toString();

    if (json.contains("block_height") && !json.value("block_height").isNull()) {
        m_blockHeight = json.value("block_height").toVariant();
    } else {
        m_blockHeight = QVariant();
    }

    if (json.contains("merkle_proof") && json.value("merkle_proof").isObject()) {
        m_merkleProof.fromJson(json.value("merkle_proof").toObject());
    } else {
        m_merkleProof = MerkleProof();
    }

    m_mmrIndex = json.value("mmr_index").toVariant().toULongLong();
}

/**
 * @brief OutputPrintable::toJson
 * @return
 */
QJsonObject OutputPrintable::toJson() const
{
    QJsonObject json;

    switch (m_outputType) {
    case OutputType::OutputTypeCoinbase:
        json["output_type"] = "Coinbase";
        break;
    case OutputType::OutputTypeTransaction:
        json["output_type"] = "Transaction";
        break;
    default:
        json["output_type"] = "Unknown";
        break;
    }

    json["commit"] = m_commit.toJson();
    json["spent"] = m_spent;

    if (!m_proof.isEmpty()) {
        json["proof"] = m_proof;
    } else {
        json["proof"] = QJsonValue::Null;
    }

    json["proof_hash"] = m_proofHash;

    if (m_blockHeight.isValid()) {
        json["block_height"] = static_cast<double>(m_blockHeight.toULongLong());
    } else {
        json["block_height"] = QJsonValue::Null;
    }

    json["merkle_proof"] = m_merkleProof.toJson();
    json["mmr_index"] = QString::number(m_mmrIndex);

    return json;
}
