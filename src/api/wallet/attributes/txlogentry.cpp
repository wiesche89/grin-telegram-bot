#include "txlogentry.h"

TxLogEntry::TxLogEntry() :
    m_id(0),
    m_confirmed(false),
    m_numInputs(0),
    m_numOutputs(0),
    m_amountCredited(0),
    m_amountDebited(0),
    m_ttlCutoffHeight(0),
    m_kernelLookupMinHeight(0),
    m_revertedAfterSeconds(0)
{
}

QString TxLogEntry::parentKeyId() const
{
    return m_parentKeyId;
}

void TxLogEntry::setParentKeyId(const QString &v)
{
    m_parentKeyId = v;
}

quint32 TxLogEntry::id() const
{
    return m_id;
}

void TxLogEntry::setId(quint32 v)
{
    m_id = v;
}

QUuid TxLogEntry::txSlateId() const
{
    return m_txSlateId;
}

void TxLogEntry::setTxSlateId(const QUuid &v)
{
    m_txSlateId = v;
}

QString TxLogEntry::txType() const
{
    return m_txType;
}

void TxLogEntry::setTxType(const QString &v)
{
    m_txType = v;
}

QDateTime TxLogEntry::creationTs() const
{
    return m_creationTs;
}

void TxLogEntry::setCreationTs(const QDateTime &v)
{
    m_creationTs = v;
}

QDateTime TxLogEntry::confirmationTs() const
{
    return m_confirmationTs;
}

void TxLogEntry::setConfirmationTs(const QDateTime &v)
{
    m_confirmationTs = v;
}

bool TxLogEntry::confirmed() const
{
    return m_confirmed;
}

void TxLogEntry::setConfirmed(bool v)
{
    m_confirmed = v;
}

int TxLogEntry::numInputs() const
{
    return m_numInputs;
}

void TxLogEntry::setNumInputs(int v)
{
    m_numInputs = v;
}

int TxLogEntry::numOutputs() const
{
    return m_numOutputs;
}

void TxLogEntry::setNumOutputs(int v)
{
    m_numOutputs = v;
}

quint64 TxLogEntry::amountCredited() const
{
    return m_amountCredited;
}

void TxLogEntry::setAmountCredited(quint64 v)
{
    m_amountCredited = v;
}

quint64 TxLogEntry::amountDebited() const
{
    return m_amountDebited;
}

void TxLogEntry::setAmountDebited(quint64 v)
{
    m_amountDebited = v;
}

QString TxLogEntry::fee() const
{
    return m_fee;
}

void TxLogEntry::setFee(const QString &v)
{
    m_fee = v;
}

quint64 TxLogEntry::ttlCutoffHeight() const
{
    return m_ttlCutoffHeight;
}

void TxLogEntry::setTtlCutoffHeight(quint64 v)
{
    m_ttlCutoffHeight = v;
}

QString TxLogEntry::storedTx() const
{
    return m_storedTx;
}

void TxLogEntry::setStoredTx(const QString &v)
{
    m_storedTx = v;
}

QString TxLogEntry::kernelExcess() const
{
    return m_kernelExcess;
}

void TxLogEntry::setKernelExcess(const QString &v)
{
    m_kernelExcess = v;
}

quint64 TxLogEntry::kernelLookupMinHeight() const
{
    return m_kernelLookupMinHeight;
}

void TxLogEntry::setKernelLookupMinHeight(quint64 v)
{
    m_kernelLookupMinHeight = v;
}

QString TxLogEntry::paymentProof() const
{
    return m_paymentProof;
}

void TxLogEntry::setPaymentProof(const QString &v)
{
    m_paymentProof = v;
}

qint64 TxLogEntry::revertedAfterSeconds() const
{
    return m_revertedAfterSeconds;
}

void TxLogEntry::setRevertedAfterSeconds(qint64 v)
{
    m_revertedAfterSeconds = v;
}

void TxLogEntry::fromJson(const QJsonObject &json)
{
    m_parentKeyId = json.value("parent_key_id").toString();
    m_id = static_cast<quint32>(json.value("id").toInt());
    m_txSlateId = QUuid(json.value("tx_slate_id").toString());
    m_txType = json.value("tx_type").toString();

    m_creationTs = QDateTime::fromString(json.value("creation_ts").toString(), Qt::ISODate);
    m_confirmationTs = QDateTime::fromString(json.value("confirmation_ts").toString(), Qt::ISODate);

    m_confirmed = json.value("confirmed").toBool();

    m_numInputs = json.value("num_inputs").toInt();
    m_numOutputs = json.value("num_outputs").toInt();
    m_amountCredited = static_cast<quint64>(json.value("amount_credited").toVariant().toULongLong());
    m_amountDebited = static_cast<quint64>(json.value("amount_debited").toVariant().toULongLong());

    m_fee = json.value("fee").toString();
    m_ttlCutoffHeight = static_cast<quint64>(json.value("ttl_cutoff_height").toVariant().toULongLong());
    m_storedTx = json.value("stored_tx").toString();
    m_kernelExcess = json.value("kernel_excess").toString();
    m_kernelLookupMinHeight = static_cast<quint64>(json.value("kernel_lookup_min_height").toVariant().toULongLong());
    m_paymentProof = json.value("payment_proof").toString();
    m_revertedAfterSeconds = json.value("reverted_after").toInt();
}

QJsonObject TxLogEntry::toJson() const
{
    QJsonObject json;
    json["parent_key_id"] = m_parentKeyId;
    json["id"] = static_cast<int>(m_id);
    json["tx_slate_id"] = m_txSlateId.toString();
    json["tx_type"] = m_txType;
    json["creation_ts"] = m_creationTs.toString(Qt::ISODate);
    json["confirmation_ts"] = m_confirmationTs.toString(Qt::ISODate);
    json["confirmed"] = m_confirmed;
    json["num_inputs"] = m_numInputs;
    json["num_outputs"] = m_numOutputs;
    json["amount_credited"] = static_cast<double>(m_amountCredited);
    json["amount_debited"] = static_cast<double>(m_amountDebited);
    json["fee"] = m_fee;
    json["ttl_cutoff_height"] = static_cast<double>(m_ttlCutoffHeight);
    json["stored_tx"] = m_storedTx;
    json["kernel_excess"] = m_kernelExcess;
    json["kernel_lookup_min_height"] = static_cast<double>(m_kernelLookupMinHeight);
    json["payment_proof"] = m_paymentProof;
    json["reverted_after"] = static_cast<int>(m_revertedAfterSeconds);

    return json;
}
