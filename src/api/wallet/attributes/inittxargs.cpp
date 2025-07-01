#include "inittxargs.h"

InitTxArgs::InitTxArgs() :
    m_srcAcctName(""),
    m_amount(0),
    m_amountIncludesFee(false),
    m_minimumConfirmations(0),
    m_maxOutputs(0),
    m_numChangeOutputs(0),
    m_selectionStrategyIsUseAll(false),
    m_targetSlateVersion(0),
    m_ttlBlocks(0),
    m_paymentProofRecipientAddress(""),
    m_estimateOnly(false),
    m_lateLock(false),
    m_sendArgs()
{
}

// Getter / Setter

QString InitTxArgs::srcAcctName() const
{
    return m_srcAcctName;
}

void InitTxArgs::setSrcAcctName(const QString &value)
{
    m_srcAcctName = value;
}

quint64 InitTxArgs::amount() const
{
    return m_amount;
}

void InitTxArgs::setAmount(quint64 value)
{
    m_amount = value;
}

bool InitTxArgs::amountIncludesFee() const
{
    return m_amountIncludesFee;
}

void InitTxArgs::setAmountIncludesFee(bool value)
{
    m_amountIncludesFee = value;
}

quint64 InitTxArgs::minimumConfirmations() const
{
    return m_minimumConfirmations;
}

void InitTxArgs::setMinimumConfirmations(quint64 value)
{
    m_minimumConfirmations = value;
}

quint32 InitTxArgs::maxOutputs() const
{
    return m_maxOutputs;
}

void InitTxArgs::setMaxOutputs(quint32 value)
{
    m_maxOutputs = value;
}

quint32 InitTxArgs::numChangeOutputs() const
{
    return m_numChangeOutputs;
}

void InitTxArgs::setNumChangeOutputs(quint32 value)
{
    m_numChangeOutputs = value;
}

bool InitTxArgs::selectionStrategyIsUseAll() const
{
    return m_selectionStrategyIsUseAll;
}

void InitTxArgs::setSelectionStrategyIsUseAll(bool value)
{
    m_selectionStrategyIsUseAll = value;
}

quint16 InitTxArgs::targetSlateVersion() const
{
    return m_targetSlateVersion;
}

void InitTxArgs::setTargetSlateVersion(quint16 value)
{
    m_targetSlateVersion = value;
}

quint64 InitTxArgs::ttlBlocks() const
{
    return m_ttlBlocks;
}

void InitTxArgs::setTtlBlocks(quint64 value)
{
    m_ttlBlocks = value;
}

QString InitTxArgs::paymentProofRecipientAddress() const
{
    return m_paymentProofRecipientAddress;
}

void InitTxArgs::setPaymentProofRecipientAddress(const QString &value)
{
    m_paymentProofRecipientAddress = value;
}

bool InitTxArgs::estimateOnly() const
{
    return m_estimateOnly;
}

void InitTxArgs::setEstimateOnly(bool value)
{
    m_estimateOnly = value;
}

bool InitTxArgs::lateLock() const
{
    return m_lateLock;
}

void InitTxArgs::setLateLock(bool value)
{
    m_lateLock = value;
}

InitTxSendArgs InitTxArgs::sendArgs() const
{
    return m_sendArgs;
}

void InitTxArgs::setSendArgs(const InitTxSendArgs &value)
{
    m_sendArgs = value;
}

// JSON

QJsonObject InitTxArgs::toJson() const
{
    QJsonObject obj;

    obj["src_acct_name"] = m_srcAcctName;
    obj["amount"] = QString::number(m_amount);
    obj["amount_includes_fee"] = m_amountIncludesFee;
    obj["minimum_confirmations"] = QString::number(m_minimumConfirmations);
    obj["max_outputs"] = static_cast<int>(m_maxOutputs);
    obj["num_change_outputs"] = static_cast<int>(m_numChangeOutputs);
    obj["selection_strategy_is_use_all"] = m_selectionStrategyIsUseAll;
    obj["target_slate_version"] = static_cast<int>(m_targetSlateVersion);
    obj["ttl_blocks"] = QString::number(m_ttlBlocks);
    obj["payment_proof_recipient_address"] = m_paymentProofRecipientAddress;
    obj["estimate_only"] = m_estimateOnly;
    obj["late_lock"] = m_lateLock;
    obj["send_args"] = m_sendArgs.toJson();

    return obj;
}

bool InitTxArgs::fromJson(const QJsonObject &json)
{
    if (json.contains("src_acct_name") && json["src_acct_name"].isString()) {
        m_srcAcctName = json["src_acct_name"].toString();
    } else {
        m_srcAcctName = "";
    }

    if (json.contains("amount")) {
        if (json["amount"].isDouble()) {
            m_amount = static_cast<quint64>(json["amount"].toDouble());
        } else if (json["amount"].isString()) {
            m_amount = json["amount"].toString().toULongLong();
        } else {
            return false;
        }
    } else {
        return false;
    }

    if (json.contains("amount_includes_fee") && json["amount_includes_fee"].isBool()) {
        m_amountIncludesFee = json["amount_includes_fee"].toBool();
    } else {
        m_amountIncludesFee = false;
    }

    if (json.contains("minimum_confirmations")) {
        if (json["minimum_confirmations"].isDouble()) {
            m_minimumConfirmations = static_cast<quint64>(json["minimum_confirmations"].toDouble());
        } else if (json["minimum_confirmations"].isString()) {
            m_minimumConfirmations = json["minimum_confirmations"].toString().toULongLong();
        } else {
            return false;
        }
    } else {
        return false;
    }

    if (json.contains("max_outputs") && json["max_outputs"].isDouble()) {
        m_maxOutputs = static_cast<quint32>(json["max_outputs"].toDouble());
    } else {
        return false;
    }

    if (json.contains("num_change_outputs") && json["num_change_outputs"].isDouble()) {
        m_numChangeOutputs = static_cast<quint32>(json["num_change_outputs"].toDouble());
    } else {
        return false;
    }

    if (json.contains("selection_strategy_is_use_all") && json["selection_strategy_is_use_all"].isBool()) {
        m_selectionStrategyIsUseAll = json["selection_strategy_is_use_all"].toBool();
    } else {
        return false;
    }

    if (json.contains("target_slate_version")) {
        if (json["target_slate_version"].isDouble()) {
            m_targetSlateVersion = static_cast<quint16>(json["target_slate_version"].toInt());
        } else {
            m_targetSlateVersion = 0;
        }
    } else {
        m_targetSlateVersion = 0;
    }

    if (json.contains("ttl_blocks")) {
        if (json["ttl_blocks"].isDouble()) {
            m_ttlBlocks = static_cast<quint64>(json["ttl_blocks"].toDouble());
        } else if (json["ttl_blocks"].isString()) {
            m_ttlBlocks = json["ttl_blocks"].toString().toULongLong();
        } else {
            m_ttlBlocks = 0;
        }
    } else {
        m_ttlBlocks = 0;
    }

    if (json.contains("payment_proof_recipient_address") && json["payment_proof_recipient_address"].isString()) {
        m_paymentProofRecipientAddress = json["payment_proof_recipient_address"].toString();
    } else {
        m_paymentProofRecipientAddress = "";
    }

    if (json.contains("estimate_only") && json["estimate_only"].isBool()) {
        m_estimateOnly = json["estimate_only"].toBool();
    } else {
        m_estimateOnly = false;
    }

    if (json.contains("late_lock") && json["late_lock"].isBool()) {
        m_lateLock = json["late_lock"].toBool();
    } else {
        m_lateLock = false;
    }

    if (json.contains("send_args") && json["send_args"].isObject()) {
        if (!m_sendArgs.fromJson(json["send_args"].toObject())) {
            return false;
        }
    } else {
        m_sendArgs = InitTxSendArgs();
    }

    return true;
}
