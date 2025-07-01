#ifndef BLOCKHEADERPRINTABLE_H
#define BLOCKHEADERPRINTABLE_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>

class BlockHeaderPrintable
{
public:
    BlockHeaderPrintable();

    // Getter / Setter
    QString hash() const;
    void setHash(const QString &hash);

    quint16 version() const;
    void setVersion(quint16 version);

    quint64 height() const;
    void setHeight(quint64 height);

    QString previous() const;
    void setPrevious(const QString &previous);

    QString prevRoot() const;
    void setPrevRoot(const QString &prevRoot);

    QString timestamp() const;
    void setTimestamp(const QString &timestamp);

    QString outputRoot() const;
    void setOutputRoot(const QString &outputRoot);

    quint64 outputMmrSize() const;
    void setOutputMmrSize(quint64 size);

    QString rangeProofRoot() const;
    void setRangeProofRoot(const QString &root);

    QString kernelRoot() const;
    void setKernelRoot(const QString &root);

    quint64 kernelMmrSize() const;
    void setKernelMmrSize(quint64 size);

    quint64 nonce() const;
    void setNonce(quint64 nonce);

    quint8 edgeBits() const;
    void setEdgeBits(quint8 bits);

    QVector<quint64> cuckooSolution() const;
    void setCuckooSolution(const QVector<quint64> &solution);

    quint64 totalDifficulty() const;
    void setTotalDifficulty(quint64 difficulty);

    quint32 secondaryScaling() const;
    void setSecondaryScaling(quint32 scaling);

    QString totalKernelOffset() const;
    void setTotalKernelOffset(const QString &offset);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QString m_hash;
    quint16 m_version;
    quint64 m_height;
    QString m_previous;
    QString m_prevRoot;
    QString m_timestamp;
    QString m_outputRoot;
    quint64 m_outputMmrSize;
    QString m_rangeProofRoot;
    QString m_kernelRoot;
    quint64 m_kernelMmrSize;
    quint64 m_nonce;
    quint8 m_edgeBits;
    QVector<quint64> m_cuckooSolution;
    quint64 m_totalDifficulty;
    quint32 m_secondaryScaling;
    QString m_totalKernelOffset;
};

#endif // BLOCKHEADERPRINTABLE_H
