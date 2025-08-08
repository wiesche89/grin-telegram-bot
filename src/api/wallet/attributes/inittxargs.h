#ifndef INITTARGS_H
#define INITTARGS_H

#include <QString>
#include <QJsonObject>

#include "inittxsendargs.h"

class InitTxArgs
{
public:
    InitTxArgs();

    QJsonValue srcAcctName() const;
    void setSrcAcctName(const QJsonValue &value);

    quint64 amount() const;
    void setAmount(quint64 value);

    QJsonValue amountIncludesFee() const;
    void setAmountIncludesFee(QJsonValue value);

    quint64 minimumConfirmations() const;
    void setMinimumConfirmations(quint64 value);

    quint32 maxOutputs() const;
    void setMaxOutputs(quint32 value);

    quint32 numChangeOutputs() const;
    void setNumChangeOutputs(quint32 value);

    bool selectionStrategyIsUseAll() const;
    void setSelectionStrategyIsUseAll(bool value);

    QJsonValue targetSlateVersion() const;
    void setTargetSlateVersion(QJsonValue value);

    QJsonValue ttlBlocks() const;
    void setTtlBlocks(QJsonValue value);

    QJsonValue paymentProofRecipientAddress() const;
    void setPaymentProofRecipientAddress(const QJsonValue &value);

    QJsonValue estimateOnly() const;
    void setEstimateOnly(QJsonValue value);

    QJsonValue lateLock() const;
    void setLateLock(QJsonValue value);

    InitTxSendArgs sendArgs() const;
    void setSendArgs(const InitTxSendArgs &value);

    // JSON
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

private:

    //Optional
    //The human readable account name from which to draw outputs for the transaction,
    //overriding whatever the active account is as set via the set_active_account method.
    QJsonValue m_srcAcctName;

    //The amount to send, in nanogrins. (1 G = 1_000_000_000nG)
    quint64 m_amount;

    //Optional
    //Does the amount include the fee, or will fees be spent in addition to the amount?
    QJsonValue m_amountIncludesFee;

    //The minimum number of confirmations an output should have in order to be included in the transaction.
    quint64 m_minimumConfirmations;

    //By default, the wallet selects as many inputs as possible in a transaction, to reduce the Output set and the fees.
    //The wallet will attempt to spend include up to max_outputs in a transaction, however if this is not enough to cover the whole amount,
    //the wallet will include more outputs. This parameter should be considered a soft limit.
    quint32 m_maxOutputs;

    //The target number of change outputs to create in the transaction.
    //The actual number created will be num_change_outputs + whatever remainder is needed.
    quint32 m_numChangeOutputs;

    //If true, attempt to use up as many outputs as possible to create the transaction, up the ‘soft limit’ of max_outputs.
    //This helps to reduce the size of the UTXO set and the amount of data stored in the wallet, and minimizes fees.
    //This will generally result in many inputs and a large change output(s), usually much larger than the amount being sent.
    //If false, the transaction will include as many outputs as are needed to meet the amount, (and no more) starting with the smallest value outputs.
    bool m_selectionStrategyIsUseAll;

    //Optional
    //Optionally set the output target slate version (acceptable down to the minimum slate version compatible with the current.
    //If None the slate is generated with the latest version.
    QJsonValue m_targetSlateVersion;

    //Optional
    //Number of blocks from current after which TX should be ignored
    QJsonValue m_ttlBlocks;

    //Optional
    //If set, require a payment proof for the particular recipient
    QJsonValue m_paymentProofRecipientAddress;

    //Optional
    //If true, just return an estimate of the resulting slate, containing fees and amounts locked without actually
    //locking outputs or creating the transaction. Note if this is set to ‘true’, the amount field in the slate will contain
    //the total amount locked, not the provided transaction amount
    QJsonValue m_estimateOnly;

    //Optional
    //EXPERIMENTAL: if flagged, create the transaction as late-locked, i.e. don’t select actual inputs until just before finalization
    QJsonValue m_lateLock;

    //Optional
    //Sender arguments. If present, the underlying function will also attempt to send the transaction to a destination and optionally finalize the result
    InitTxSendArgs m_sendArgs;
};

#endif // INITTARGS_H
