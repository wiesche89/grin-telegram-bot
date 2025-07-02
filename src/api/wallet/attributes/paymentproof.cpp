#include "paymentproof.h"

/**
 * @brief PaymentProof::PaymentProof
 */
PaymentProof::PaymentProof() :
    m_amount(0)
{
}

/**
 * @brief PaymentProof::amount
 * @return
 */
quint64 PaymentProof::amount() const
{
    return m_amount;
}

/**
 * @brief PaymentProof::setAmount
 * @param amount
 */
void PaymentProof::setAmount(quint64 amount)
{
    m_amount = amount;
}

/**
 * @brief PaymentProof::excess
 * @return
 */
Commitment PaymentProof::excess() const
{
    return m_excess;
}

/**
 * @brief PaymentProof::setExcess
 * @param excess
 */
void PaymentProof::setExcess(const Commitment &excess)
{
    m_excess = excess;
}

/**
 * @brief PaymentProof::recipientAddress
 * @return
 */
QString PaymentProof::recipientAddress() const
{
    return m_recipientAddress;
}

/**
 * @brief PaymentProof::setRecipientAddress
 * @param address
 */
void PaymentProof::setRecipientAddress(const QString &address)
{
    m_recipientAddress = address;
}

/**
 * @brief PaymentProof::recipientSignature
 * @return
 */
QString PaymentProof::recipientSignature() const
{
    return m_recipientSig;
}

/**
 * @brief PaymentProof::setRecipientSignature
 * @param signature
 */
void PaymentProof::setRecipientSignature(const QString &signature)
{
    m_recipientSig = signature;
}

/**
 * @brief PaymentProof::senderAddress
 * @return
 */
QString PaymentProof::senderAddress() const
{
    return m_senderAddress;
}

/**
 * @brief PaymentProof::setSenderAddress
 * @param address
 */
void PaymentProof::setSenderAddress(const QString &address)
{
    m_senderAddress = address;
}

/**
 * @brief PaymentProof::senderSignature
 * @return
 */
QString PaymentProof::senderSignature() const
{
    return m_senderSig;
}

/**
 * @brief PaymentProof::setSenderSignature
 * @param signature
 */
void PaymentProof::setSenderSignature(const QString &signature)
{
    m_senderSig = signature;
}

/**
 * @brief PaymentProof::fromJson
 * @param obj
 * @return
 */
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

/**
 * @brief PaymentProof::toJson
 * @return
 */
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
