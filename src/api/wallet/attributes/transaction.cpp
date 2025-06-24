#include "transaction.h"

Transaction::Transaction() :
    m_amountCredited(0),
    m_amountDebited(0),
    m_confirmed(false),
    m_id(-1),
    m_kernelLookupMinHeight(0),
    m_numInputs(0),
    m_numOutputs(0)
{
}

// Getter
qint64 Transaction::amountCredited() const
{
    return m_amountCredited;
}

qint64 Transaction::amountDebited() const
{
    return m_amountDebited;
}

QString Transaction::confirmationTimestamp() const
{
    return m_confirmationTs;
}

bool Transaction::isConfirmed() const
{
    return m_confirmed;
}

QString Transaction::creationTimestamp() const
{
    return m_creationTs;
}

QString Transaction::fee() const
{
    return m_fee;
}

int Transaction::id() const
{
    return m_id;
}

QString Transaction::kernelExcess() const
{
    return m_kernelExcess;
}

int Transaction::kernelLookupMinHeight() const
{
    return m_kernelLookupMinHeight;
}

int Transaction::numInputs() const
{
    return m_numInputs;
}

int Transaction::numOutputs() const
{
    return m_numOutputs;
}

QString Transaction::parentKeyId() const
{
    return m_parentKeyId;
}

QString Transaction::txSlateId() const
{
    return m_txSlateId;
}

QString Transaction::txType() const
{
    return m_txType;
}

// Setter
void Transaction::setAmountCredited(qint64 value)
{
    m_amountCredited = value;
}

void Transaction::setAmountDebited(qint64 value)
{
    m_amountDebited = value;
}

void Transaction::setConfirmationTimestamp(const QString &value)
{
    m_confirmationTs = value;
}

void Transaction::setConfirmed(bool value)
{
    m_confirmed = value;
}

void Transaction::setCreationTimestamp(const QString &value)
{
    m_creationTs = value;
}

void Transaction::setFee(const QString &value)
{
    m_fee = value;
}

void Transaction::setId(int value)
{
    m_id = value;
}

void Transaction::setKernelExcess(const QString &value)
{
    m_kernelExcess = value;
}

void Transaction::setKernelLookupMinHeight(int value)
{
    m_kernelLookupMinHeight = value;
}

void Transaction::setNumInputs(int value)
{
    m_numInputs = value;
}

void Transaction::setNumOutputs(int value)
{
    m_numOutputs = value;
}

void Transaction::setParentKeyId(const QString &value)
{
    m_parentKeyId = value;
}

void Transaction::setTxSlateId(const QString &value)
{
    m_txSlateId = value;
}

void Transaction::setTxType(const QString &value)
{
    m_txType = value;
}

// JSON
QJsonObject Transaction::toJson() const
{
    QJsonObject obj;
    obj["amount_credited"] = QString::number(m_amountCredited);
    obj["amount_debited"] = QString::number(m_amountDebited);
    obj["confirmation_ts"] = m_confirmationTs;
    obj["confirmed"] = m_confirmed;
    obj["creation_ts"] = m_creationTs;
    obj["fee"] = m_fee;
    obj["id"] = m_id;
    obj["kernel_excess"] = m_kernelExcess;
    obj["kernel_lookup_min_height"] = m_kernelLookupMinHeight;
    obj["num_inputs"] = m_numInputs;
    obj["num_outputs"] = m_numOutputs;
    obj["parent_key_id"] = m_parentKeyId;
    obj["tx_slate_id"] = m_txSlateId;
    obj["tx_type"] = m_txType;
    return obj;
}

void Transaction::fromJson(const QJsonObject &obj)
{
    m_amountCredited = obj["amount_credited"].toString().toLongLong();
    m_amountDebited = obj["amount_debited"].toString().toLongLong();
    m_confirmationTs = obj["confirmation_ts"].toString();
    m_confirmed = obj["confirmed"].toBool();
    m_creationTs = obj["creation_ts"].toString();
    m_fee = obj["fee"].toString();
    m_id = obj["id"].toInt();
    m_kernelExcess = obj["kernel_excess"].toString();
    m_kernelLookupMinHeight = obj["kernel_lookup_min_height"].toInt();
    m_numInputs = obj["num_inputs"].toInt();
    m_numOutputs = obj["num_outputs"].toInt();
    m_parentKeyId = obj["parent_key_id"].toString();
    m_txSlateId = obj["tx_slate_id"].toString();
    m_txType = obj["tx_type"].toString();
}
