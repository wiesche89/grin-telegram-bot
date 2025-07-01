#ifndef PAYMENTPROOF_H
#define PAYMENTPROOF_H

#include <QString>
#include <QJsonObject>
#include "Commitment.h"

class PaymentProof
{
public:
    PaymentProof();

    quint64 amount() const;
    void setAmount(quint64 amount);

    Commitment excess() const;
    void setExcess(const Commitment &excess);

    QString recipientAddress() const;
    void setRecipientAddress(const QString &address);

    QString recipientSignature() const;
    void setRecipientSignature(const QString &signature);

    QString senderAddress() const;
    void setSenderAddress(const QString &address);

    QString senderSignature() const;
    void setSenderSignature(const QString &signature);

    bool fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    quint64 m_amount;
    Commitment m_excess;
    QString m_recipientAddress;
    QString m_recipientSig;
    QString m_senderAddress;
    QString m_senderSig;
};

#endif // PAYMENTPROOF_H
