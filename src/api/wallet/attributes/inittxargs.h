#ifndef INITTARGS_H
#define INITTARGS_H

#include <QString>
#include <QJsonObject>
#include "inittxsendargs.h"

class InitTxArgs
{
public:
    InitTxArgs();

    // Getter / Setter
    QString srcAcctName() const;
    void setSrcAcctName(const QString &value);

    quint64 amount() const;
    void setAmount(quint64 value);

    bool amountIncludesFee() const;
    void setAmountIncludesFee(bool value);

    quint64 minimumConfirmations() const;
    void setMinimumConfirmations(quint64 value);

    quint32 maxOutputs() const;
    void setMaxOutputs(quint32 value);

    quint32 numChangeOutputs() const;
    void setNumChangeOutputs(quint32 value);

    bool selectionStrategyIsUseAll() const;
    void setSelectionStrategyIsUseAll(bool value);

    quint16 targetSlateVersion() const;
    void setTargetSlateVersion(quint16 value);

    quint64 ttlBlocks() const;
    void setTtlBlocks(quint64 value);

    QString paymentProofRecipientAddress() const;
    void setPaymentProofRecipientAddress(const QString &value);

    bool estimateOnly() const;
    void setEstimateOnly(bool value);

    bool lateLock() const;
    void setLateLock(bool value);

    InitTxSendArgs sendArgs() const;
    void setSendArgs(const InitTxSendArgs &value);

    // JSON
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

private:
    QString m_srcAcctName;
    quint64 m_amount;
    bool m_amountIncludesFee;

    quint64 m_minimumConfirmations;
    quint32 m_maxOutputs;
    quint32 m_numChangeOutputs;
    bool m_selectionStrategyIsUseAll;

    quint16 m_targetSlateVersion;
    quint64 m_ttlBlocks;

    QString m_paymentProofRecipientAddress;

    bool m_estimateOnly;
    bool m_lateLock;

    InitTxSendArgs m_sendArgs;
};

#endif // INITTARGS_H
