#include "verifypaymentproofstatus.h"

VerifyPaymentProofStatus::VerifyPaymentProofStatus() :
    senderBelongs(false),
    recipientBelongs(false)
{
}

VerifyPaymentProofStatus::VerifyPaymentProofStatus(bool sender, bool recipient) :
    senderBelongs(sender),
    recipientBelongs(recipient)
{
}

bool VerifyPaymentProofStatus::senderBelongsToWallet() const
{
    return senderBelongs;
}

void VerifyPaymentProofStatus::setSenderBelongsToWallet(bool val)
{
    senderBelongs = val;
}

bool VerifyPaymentProofStatus::recipientBelongsToWallet() const
{
    return recipientBelongs;
}

void VerifyPaymentProofStatus::setRecipientBelongsToWallet(bool val)
{
    recipientBelongs = val;
}

void VerifyPaymentProofStatus::fromJson(const QJsonObject &json)
{
    if (json.contains("sender") && json["sender"].isBool()) {
        senderBelongs = json["sender"].toBool();
    }

    if (json.contains("recipient") && json["recipient"].isBool()) {
        recipientBelongs = json["recipient"].toBool();
    }
}

QJsonObject VerifyPaymentProofStatus::toJson() const
{
    QJsonObject json;
    json["sender"] = senderBelongs;
    json["recipient"] = recipientBelongs;
    return json;
}
