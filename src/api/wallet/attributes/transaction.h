#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QJsonObject>
#include <QDebug>

class Transaction
{
public:
    Transaction();

    // Getter
    qint64 amountCredited() const;
    qint64 amountDebited() const;
    QString confirmationTimestamp() const;
    bool isConfirmed() const;
    QString creationTimestamp() const;
    QString fee() const;
    int id() const;
    QString kernelExcess() const;
    int kernelLookupMinHeight() const;
    int numInputs() const;
    int numOutputs() const;
    QString parentKeyId() const;
    QString txSlateId() const;
    QString txType() const;

    // Setter
    void setAmountCredited(qint64 value);
    void setAmountDebited(qint64 value);
    void setConfirmationTimestamp(const QString &value);
    void setConfirmed(bool value);
    void setCreationTimestamp(const QString &value);
    void setFee(const QString &value);
    void setId(int value);
    void setKernelExcess(const QString &value);
    void setKernelLookupMinHeight(int value);
    void setNumInputs(int value);
    void setNumOutputs(int value);
    void setParentKeyId(const QString &value);
    void setTxSlateId(const QString &value);
    void setTxType(const QString &value);

    // JSON handling
    QJsonObject toJson() const;
    static Transaction fromJson(const QJsonObject &obj);

private:
    qint64 m_amountCredited;
    qint64 m_amountDebited;
    QString m_confirmationTs;
    bool m_confirmed;
    QString m_creationTs;
    QString m_fee;
    int m_id;
    QString m_kernelExcess;
    int m_kernelLookupMinHeight;
    int m_numInputs;
    int m_numOutputs;
    QString m_parentKeyId;
    QString m_txSlateId;
    QString m_txType;
};

#endif // TRANSACTION_H
