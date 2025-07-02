#ifndef VERIFYPAYMENTPROOFSTATUS_H
#define VERIFYPAYMENTPROOFSTATUS_H

#include <QJsonObject>

class VerifyPaymentProofStatus
{
public:
    VerifyPaymentProofStatus();
    VerifyPaymentProofStatus(bool sender, bool recipient);

    bool senderBelongsToWallet() const;
    void setSenderBelongsToWallet(bool val);

    bool recipientBelongsToWallet() const;
    void setRecipientBelongsToWallet(bool val);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    bool m_senderBelongs;
    bool m_recipientBelongs;
};

#endif // VERIFYPAYMENTPROOFSTATUS_H
