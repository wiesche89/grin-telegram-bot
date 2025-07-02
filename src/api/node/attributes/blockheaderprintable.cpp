#include "blockheaderprintable.h"

BlockHeaderPrintable::BlockHeaderPrintable() :
    m_version(0),
    m_height(0),
    m_outputMmrSize(0),
    m_kernelMmrSize(0),
    m_nonce(0),
    m_edgeBits(0),
    m_totalDifficulty(0),
    m_secondaryScaling(0)
{
}

/**
 * @brief BlockHeaderPrintable::hash
 * @return
 */
QString BlockHeaderPrintable::hash() const
{
    return m_hash;
}

/**
 * @brief BlockHeaderPrintable::setHash
 * @param hash
 */
void BlockHeaderPrintable::setHash(const QString &hash)
{
    m_hash = hash;
}

/**
 * @brief BlockHeaderPrintable::version
 * @return
 */
quint16 BlockHeaderPrintable::version() const
{
    return m_version;
}

/**
 * @brief BlockHeaderPrintable::setVersion
 * @param version
 */
void BlockHeaderPrintable::setVersion(quint16 version)
{
    m_version = version;
}

/**
 * @brief BlockHeaderPrintable::height
 * @return
 */
quint64 BlockHeaderPrintable::height() const
{
    return m_height;
}

/**
 * @brief BlockHeaderPrintable::setHeight
 * @param height
 */
void BlockHeaderPrintable::setHeight(quint64 height)
{
    m_height = height;
}

/**
 * @brief BlockHeaderPrintable::previous
 * @return
 */
QString BlockHeaderPrintable::previous() const
{
    return m_previous;
}

/**
 * @brief BlockHeaderPrintable::setPrevious
 * @param previous
 */
void BlockHeaderPrintable::setPrevious(const QString &previous)
{
    m_previous = previous;
}

/**
 * @brief BlockHeaderPrintable::prevRoot
 * @return
 */
QString BlockHeaderPrintable::prevRoot() const
{
    return m_prevRoot;
}

/**
 * @brief BlockHeaderPrintable::setPrevRoot
 * @param prevRoot
 */
void BlockHeaderPrintable::setPrevRoot(const QString &prevRoot)
{
    m_prevRoot = prevRoot;
}

/**
 * @brief BlockHeaderPrintable::timestamp
 * @return
 */
QString BlockHeaderPrintable::timestamp() const
{
    return m_timestamp;
}

/**
 * @brief BlockHeaderPrintable::setTimestamp
 * @param timestamp
 */
void BlockHeaderPrintable::setTimestamp(const QString &timestamp)
{
    m_timestamp = timestamp;
}

/**
 * @brief BlockHeaderPrintable::outputRoot
 * @return
 */
QString BlockHeaderPrintable::outputRoot() const
{
    return m_outputRoot;
}

/**
 * @brief BlockHeaderPrintable::setOutputRoot
 * @param outputRoot
 */
void BlockHeaderPrintable::setOutputRoot(const QString &outputRoot)
{
    m_outputRoot = outputRoot;
}

/**
 * @brief BlockHeaderPrintable::outputMmrSize
 * @return
 */
quint64 BlockHeaderPrintable::outputMmrSize() const
{
    return m_outputMmrSize;
}

/**
 * @brief BlockHeaderPrintable::setOutputMmrSize
 * @param size
 */
void BlockHeaderPrintable::setOutputMmrSize(quint64 size)
{
    m_outputMmrSize = size;
}

/**
 * @brief BlockHeaderPrintable::rangeProofRoot
 * @return
 */
QString BlockHeaderPrintable::rangeProofRoot() const
{
    return m_rangeProofRoot;
}

/**
 * @brief BlockHeaderPrintable::setRangeProofRoot
 * @param root
 */
void BlockHeaderPrintable::setRangeProofRoot(const QString &root)
{
    m_rangeProofRoot = root;
}

/**
 * @brief BlockHeaderPrintable::kernelRoot
 * @return
 */
QString BlockHeaderPrintable::kernelRoot() const
{
    return m_kernelRoot;
}

/**
 * @brief BlockHeaderPrintable::setKernelRoot
 * @param root
 */
void BlockHeaderPrintable::setKernelRoot(const QString &root)
{
    m_kernelRoot = root;
}

/**
 * @brief BlockHeaderPrintable::kernelMmrSize
 * @return
 */
quint64 BlockHeaderPrintable::kernelMmrSize() const
{
    return m_kernelMmrSize;
}

void BlockHeaderPrintable::setKernelMmrSize(quint64 size)
{
    m_kernelMmrSize = size;
}

/**
 * @brief BlockHeaderPrintable::nonce
 * @return
 */
quint64 BlockHeaderPrintable::nonce() const
{
    return m_nonce;
}

/**
 * @brief BlockHeaderPrintable::setNonce
 * @param nonce
 */
void BlockHeaderPrintable::setNonce(quint64 nonce)
{
    m_nonce = nonce;
}

/**
 * @brief BlockHeaderPrintable::edgeBits
 * @return
 */
quint8 BlockHeaderPrintable::edgeBits() const
{
    return m_edgeBits;
}

/**
 * @brief BlockHeaderPrintable::setEdgeBits
 * @param bits
 */
void BlockHeaderPrintable::setEdgeBits(quint8 bits)
{
    m_edgeBits = bits;
}

/**
 * @brief BlockHeaderPrintable::cuckooSolution
 * @return
 */
QVector<quint64> BlockHeaderPrintable::cuckooSolution() const
{
    return m_cuckooSolution;
}

/**
 * @brief BlockHeaderPrintable::setCuckooSolution
 * @param solution
 */
void BlockHeaderPrintable::setCuckooSolution(const QVector<quint64> &solution)
{
    m_cuckooSolution = solution;
}

/**
 * @brief BlockHeaderPrintable::totalDifficulty
 * @return
 */
quint64 BlockHeaderPrintable::totalDifficulty() const
{
    return m_totalDifficulty;
}

/**
 * @brief BlockHeaderPrintable::setTotalDifficulty
 * @param difficulty
 */
void BlockHeaderPrintable::setTotalDifficulty(quint64 difficulty)
{
    m_totalDifficulty = difficulty;
}

/**
 * @brief BlockHeaderPrintable::secondaryScaling
 * @return
 */
quint32 BlockHeaderPrintable::secondaryScaling() const
{
    return m_secondaryScaling;
}

/**
 * @brief BlockHeaderPrintable::setSecondaryScaling
 * @param scaling
 */
void BlockHeaderPrintable::setSecondaryScaling(quint32 scaling)
{
    m_secondaryScaling = scaling;
}

/**
 * @brief BlockHeaderPrintable::totalKernelOffset
 * @return
 */
QString BlockHeaderPrintable::totalKernelOffset() const
{
    return m_totalKernelOffset;
}

/**
 * @brief BlockHeaderPrintable::setTotalKernelOffset
 * @param offset
 */
void BlockHeaderPrintable::setTotalKernelOffset(const QString &offset)
{
    m_totalKernelOffset = offset;
}

/**
 * @brief BlockHeaderPrintable::fromJson
 * @param json
 */
void BlockHeaderPrintable::fromJson(const QJsonObject &json)
{
    m_hash = json.value("hash").toString();
    m_version = static_cast<quint16>(json.value("version").toInt());
    m_height = json.value("height").toVariant().toULongLong();
    m_previous = json.value("previous").toString();
    m_prevRoot = json.value("prev_root").toString();
    m_timestamp = json.value("timestamp").toString();
    m_outputRoot = json.value("output_root").toString();
    m_outputMmrSize = json.value("output_mmr_size").toVariant().toULongLong();
    m_rangeProofRoot = json.value("range_proof_root").toString();
    m_kernelRoot = json.value("kernel_root").toString();
    m_kernelMmrSize = json.value("kernel_mmr_size").toVariant().toULongLong();
    m_nonce = json.value("nonce").toVariant().toULongLong();
    m_edgeBits = static_cast<quint8>(json.value("edge_bits").toInt());

    m_cuckooSolution.clear();
    if (json.contains("cuckoo_solution") && json["cuckoo_solution"].isArray()) {
        QJsonArray arr = json["cuckoo_solution"].toArray();
        for (const QJsonValue &val : arr) {
            m_cuckooSolution.append(static_cast<quint64>(val.toVariant().toULongLong()));
        }
    }

    m_totalDifficulty = json.value("total_difficulty").toVariant().toULongLong();
    m_secondaryScaling = static_cast<quint32>(json.value("secondary_scaling").toInt());
    m_totalKernelOffset = json.value("total_kernel_offset").toString();
}

/**
 * @brief BlockHeaderPrintable::toJson
 * @return
 */
QJsonObject BlockHeaderPrintable::toJson() const
{
    QJsonObject json;
    json["hash"] = m_hash;
    json["version"] = static_cast<int>(m_version);
    json["height"] = QString::number(m_height);
    json["previous"] = m_previous;
    json["prev_root"] = m_prevRoot;
    json["timestamp"] = m_timestamp;
    json["output_root"] = m_outputRoot;
    json["output_mmr_size"] = QString::number(m_outputMmrSize);
    json["range_proof_root"] = m_rangeProofRoot;
    json["kernel_root"] = m_kernelRoot;
    json["kernel_mmr_size"] = QString::number(m_kernelMmrSize);
    json["nonce"] = QString::number(m_nonce);
    json["edge_bits"] = static_cast<int>(m_edgeBits);

    QJsonArray arr;
    for (quint64 v : m_cuckooSolution) {
        arr.append(QString::number(v));
    }
    json["cuckoo_solution"] = arr;

    json["total_difficulty"] = QString::number(m_totalDifficulty);
    json["secondary_scaling"] = static_cast<int>(m_secondaryScaling);
    json["total_kernel_offset"] = m_totalKernelOffset;

    return json;
}
