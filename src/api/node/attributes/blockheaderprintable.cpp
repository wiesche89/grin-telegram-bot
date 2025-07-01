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

// Getter / Setter Implementierungen

QString BlockHeaderPrintable::hash() const
{
    return m_hash;
}

void BlockHeaderPrintable::setHash(const QString &hash)
{
    m_hash = hash;
}

quint16 BlockHeaderPrintable::version() const
{
    return m_version;
}

void BlockHeaderPrintable::setVersion(quint16 version)
{
    m_version = version;
}

quint64 BlockHeaderPrintable::height() const
{
    return m_height;
}

void BlockHeaderPrintable::setHeight(quint64 height)
{
    m_height = height;
}

QString BlockHeaderPrintable::previous() const
{
    return m_previous;
}

void BlockHeaderPrintable::setPrevious(const QString &previous)
{
    m_previous = previous;
}

QString BlockHeaderPrintable::prevRoot() const
{
    return m_prevRoot;
}

void BlockHeaderPrintable::setPrevRoot(const QString &prevRoot)
{
    m_prevRoot = prevRoot;
}

QString BlockHeaderPrintable::timestamp() const
{
    return m_timestamp;
}

void BlockHeaderPrintable::setTimestamp(const QString &timestamp)
{
    m_timestamp = timestamp;
}

QString BlockHeaderPrintable::outputRoot() const
{
    return m_outputRoot;
}

void BlockHeaderPrintable::setOutputRoot(const QString &outputRoot)
{
    m_outputRoot = outputRoot;
}

quint64 BlockHeaderPrintable::outputMmrSize() const
{
    return m_outputMmrSize;
}

void BlockHeaderPrintable::setOutputMmrSize(quint64 size)
{
    m_outputMmrSize = size;
}

QString BlockHeaderPrintable::rangeProofRoot() const
{
    return m_rangeProofRoot;
}

void BlockHeaderPrintable::setRangeProofRoot(const QString &root)
{
    m_rangeProofRoot = root;
}

QString BlockHeaderPrintable::kernelRoot() const
{
    return m_kernelRoot;
}

void BlockHeaderPrintable::setKernelRoot(const QString &root)
{
    m_kernelRoot = root;
}

quint64 BlockHeaderPrintable::kernelMmrSize() const
{
    return m_kernelMmrSize;
}

void BlockHeaderPrintable::setKernelMmrSize(quint64 size)
{
    m_kernelMmrSize = size;
}

quint64 BlockHeaderPrintable::nonce() const
{
    return m_nonce;
}

void BlockHeaderPrintable::setNonce(quint64 nonce)
{
    m_nonce = nonce;
}

quint8 BlockHeaderPrintable::edgeBits() const
{
    return m_edgeBits;
}

void BlockHeaderPrintable::setEdgeBits(quint8 bits)
{
    m_edgeBits = bits;
}

QVector<quint64> BlockHeaderPrintable::cuckooSolution() const
{
    return m_cuckooSolution;
}

void BlockHeaderPrintable::setCuckooSolution(const QVector<quint64> &solution)
{
    m_cuckooSolution = solution;
}

quint64 BlockHeaderPrintable::totalDifficulty() const
{
    return m_totalDifficulty;
}

void BlockHeaderPrintable::setTotalDifficulty(quint64 difficulty)
{
    m_totalDifficulty = difficulty;
}

quint32 BlockHeaderPrintable::secondaryScaling() const
{
    return m_secondaryScaling;
}

void BlockHeaderPrintable::setSecondaryScaling(quint32 scaling)
{
    m_secondaryScaling = scaling;
}

QString BlockHeaderPrintable::totalKernelOffset() const
{
    return m_totalKernelOffset;
}

void BlockHeaderPrintable::setTotalKernelOffset(const QString &offset)
{
    m_totalKernelOffset = offset;
}

// JSON deserialization
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

// JSON serialization
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
