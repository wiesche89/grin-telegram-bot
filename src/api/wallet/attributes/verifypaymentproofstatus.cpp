#include "verifypaymentproofstatus.h"

/**
 * @brief VerifyPaymentProofStatus::VerifyPaymentProofStatus
 */
VerifyPaymentProofStatus::VerifyPaymentProofStatus() :
    m_senderBelongs(false),
    m_recipientBelongs(false)
{
}

/**
 * @brief VerifyPaymentProofStatus::VerifyPaymentProofStatus
 * @param sender
 * @param recipient
 */
VerifyPaymentProofStatus::VerifyPaymentProofStatus(bool sender, bool recipient) :
    m_senderBelongs(sender),
    m_recipientBelongs(recipient)
{
}

/**
 * @brief VerifyPaymentProofStatus::senderBelongsToWallet
 * @return
 */
bool VerifyPaymentProofStatus::senderBelongsToWallet() const
{
    return m_senderBelongs;
}

/**
 * @brief VerifyPaymentProofStatus::setSenderBelongsToWallet
 * @param val
 */
void VerifyPaymentProofStatus::setSenderBelongsToWallet(bool val)
{
    m_senderBelongs = val;
}

/**
 * @brief VerifyPaymentProofStatus::recipientBelongsToWallet
 * @return
 */
bool VerifyPaymentProofStatus::recipientBelongsToWallet() const
{
    return m_recipientBelongs;
}

/**
 * @brief VerifyPaymentProofStatus::setRecipientBelongsToWallet
 * @param val
 */
void VerifyPaymentProofStatus::setRecipientBelongsToWallet(bool val)
{
    m_recipientBelongs = val;
}

/**
 * @brief VerifyPaymentProofStatus::fromJson
 * @param json
 */
void VerifyPaymentProofStatus::fromJson(const QJsonObject &json)
{
    if (json.contains("sender") && json["sender"].isBool()) {
        m_senderBelongs = json["sender"].toBool();
    }

    if (json.contains("recipient") && json["recipient"].isBool()) {
        m_recipientBelongs = json["recipient"].toBool();
    }
}

/**
 * @brief VerifyPaymentProofStatus::toJson
 * @return
 */
QJsonObject VerifyPaymentProofStatus::toJson() const
{
    QJsonObject json;
    json["sender"] = m_senderBelongs;
    json["recipient"] = m_recipientBelongs;
    return json;
}
