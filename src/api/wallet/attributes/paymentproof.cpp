#include "paymentproof.h"

PaymentProof::PaymentProof() :
    m_amount(0)
{
}

quint64 PaymentProof::amount() const
{
    return m_amount;
}

void PaymentProof::setAmount(quint64 amount)
{
    m_amount = amount;
}

Commitment PaymentProof::excess() const
{
    return m_excess;
}

void PaymentProof::setExcess(const Commitment &excess)
{
    m_excess = excess;
}

QString PaymentProof::recipientAddress() const
{
    return m_recipientAddress;
}

void PaymentProof::setRecipientAddress(const QString &address)
{
    m_recipientAddress = address;
}

QString PaymentProof::recipientSignature() const
{
    return m_recipientSig;
}

void PaymentProof::setRecipientSignature(const QString &signature)
{
    m_recipientSig = signature;
}

QString PaymentProof::senderAddress() const
{
    return m_senderAddress;
}

void PaymentProof::setSenderAddress(const QString &address)
{
    m_senderAddress = address;
}

QString PaymentProof::senderSignature() const
{
    return m_senderSig;
}

void PaymentProof::setSenderSignature(const QString &signature)
{
    m_senderSig = signature;
}

bool PaymentProof::fromJson(const QJsonObject &obj)
{
    if (!obj.contains("amount") || !obj.contains("excess")
        || !obj.contains("recipient_address") || !obj.contains("recipient_sig")
        || !obj.contains("sender_address") || !obj.contains("sender_sig")) {
        return false;
    }

    bool ok = false;
    m_amount = obj["amount"].toString().toULongLong(&ok);
    if (!ok) {
        return false;
    }

    m_excess.fromJson(obj["excess"].toObject());
    m_recipientAddress = obj["recipient_address"].toString();
    m_recipientSig = obj["recipient_sig"].toString();
    m_senderAddress = obj["sender_address"].toString();
    m_senderSig = obj["sender_sig"].toString();

    return true;
}

QJsonObject PaymentProof::toJson() const
{
    QJsonObject obj;
    obj["amount"] = QString::number(m_amount);
    obj["excess"] = m_excess.toJson();
    obj["recipient_address"] = m_recipientAddress;
    obj["recipient_sig"] = m_recipientSig;
    obj["sender_address"] = m_senderAddress;
    obj["sender_sig"] = m_senderSig;
    return obj;
}
