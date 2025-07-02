#ifndef TXLOGENTRY_H
#define TXLOGENTRY_H

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include <QDebug>
#include <QJsonDocument>

class TxLogEntry
{
public:
    TxLogEntry();

    // Getter / Setter
    QString parentKeyId() const;
    void setParentKeyId(const QString &parentKeyId);

    quint32 id() const;
    void setId(quint32 id);

    QUuid txSlateId() const;
    void setTxSlateId(const QUuid &txSlateId);

    QString txType() const;
    void setTxType(const QString &txType);

    QDateTime creationTs() const;
    void setCreationTs(const QDateTime &creationTs);

    QDateTime confirmationTs() const;
    void setConfirmationTs(const QDateTime &confirmationTs);

    bool confirmed() const;
    void setConfirmed(bool confirmed);

    int numInputs() const;
    void setNumInputs(int numInputs);

    int numOutputs() const;
    void setNumOutputs(int numOutputs);

    quint64 amountCredited() const;
    void setAmountCredited(quint64 amountCredited);

    quint64 amountDebited() const;
    void setAmountDebited(quint64 amountDebited);

    QString fee() const;
    void setFee(const QString &fee);

    quint64 ttlCutoffHeight() const;
    void setTtlCutoffHeight(quint64 ttlCutoffHeight);

    QString storedTx() const;
    void setStoredTx(const QString &storedTx);

    QString kernelExcess() const;
    void setKernelExcess(const QString &kernelExcess);

    quint64 kernelLookupMinHeight() const;
    void setKernelLookupMinHeight(quint64 kernelLookupMinHeight);

    QString paymentProof() const;
    void setPaymentProof(const QString &paymentProof);

    qint64 revertedAfterSeconds() const;
    void setRevertedAfterSeconds(qint64 revertedAfterSeconds);

    // JSON
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QString m_parentKeyId;
    quint32 m_id;
    QUuid m_txSlateId;
    QString m_txType;
    QDateTime m_creationTs;
    QDateTime m_confirmationTs;
    bool m_confirmed;
    int m_numInputs;
    int m_numOutputs;
    quint64 m_amountCredited;
    quint64 m_amountDebited;
    QString m_fee;
    quint64 m_ttlCutoffHeight;
    QString m_storedTx;
    QString m_kernelExcess;
    quint64 m_kernelLookupMinHeight;
    QString m_paymentProof;
    qint64 m_revertedAfterSeconds;
};

#endif // TXLOGENTRY_H
