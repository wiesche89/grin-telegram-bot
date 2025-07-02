#include "txlogentry.h"

/**
 * @brief TxLogEntry::TxLogEntry
 */
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

/**
 * @brief TxLogEntry::parentKeyId
 * @return
 */
QString TxLogEntry::parentKeyId() const
{
    return m_parentKeyId;
}

/**
 * @brief TxLogEntry::setParentKeyId
 * @param v
 */
void TxLogEntry::setParentKeyId(const QString &v)
{
    m_parentKeyId = v;
}

/**
 * @brief TxLogEntry::id
 * @return
 */
quint32 TxLogEntry::id() const
{
    return m_id;
}

/**
 * @brief TxLogEntry::setId
 * @param v
 */
void TxLogEntry::setId(quint32 v)
{
    m_id = v;
}

/**
 * @brief TxLogEntry::txSlateId
 * @return
 */
QUuid TxLogEntry::txSlateId() const
{
    return m_txSlateId;
}

/**
 * @brief TxLogEntry::setTxSlateId
 * @param v
 */
void TxLogEntry::setTxSlateId(const QUuid &v)
{
    m_txSlateId = v;
}

/**
 * @brief TxLogEntry::txType
 * @return
 */
QString TxLogEntry::txType() const
{
    return m_txType;
}

/**
 * @brief TxLogEntry::setTxType
 * @param v
 */
void TxLogEntry::setTxType(const QString &v)
{
    m_txType = v;
}

/**
 * @brief TxLogEntry::creationTs
 * @return
 */
QDateTime TxLogEntry::creationTs() const
{
    return m_creationTs;
}

/**
 * @brief TxLogEntry::setCreationTs
 * @param v
 */
void TxLogEntry::setCreationTs(const QDateTime &v)
{
    m_creationTs = v;
}

/**
 * @brief TxLogEntry::confirmationTs
 * @return
 */
QDateTime TxLogEntry::confirmationTs() const
{
    return m_confirmationTs;
}

/**
 * @brief TxLogEntry::setConfirmationTs
 * @param v
 */
void TxLogEntry::setConfirmationTs(const QDateTime &v)
{
    m_confirmationTs = v;
}

/**
 * @brief TxLogEntry::confirmed
 * @return
 */
bool TxLogEntry::confirmed() const
{
    return m_confirmed;
}

/**
 * @brief TxLogEntry::setConfirmed
 * @param v
 */
void TxLogEntry::setConfirmed(bool v)
{
    m_confirmed = v;
}

/**
 * @brief TxLogEntry::numInputs
 * @return
 */
int TxLogEntry::numInputs() const
{
    return m_numInputs;
}

/**
 * @brief TxLogEntry::setNumInputs
 * @param v
 */
void TxLogEntry::setNumInputs(int v)
{
    m_numInputs = v;
}

/**
 * @brief TxLogEntry::numOutputs
 * @return
 */
int TxLogEntry::numOutputs() const
{
    return m_numOutputs;
}

/**
 * @brief TxLogEntry::setNumOutputs
 * @param v
 */
void TxLogEntry::setNumOutputs(int v)
{
    m_numOutputs = v;
}

/**
 * @brief TxLogEntry::amountCredited
 * @return
 */
quint64 TxLogEntry::amountCredited() const
{
    return m_amountCredited;
}

/**
 * @brief TxLogEntry::setAmountCredited
 * @param v
 */
void TxLogEntry::setAmountCredited(quint64 v)
{
    m_amountCredited = v;
}

/**
 * @brief TxLogEntry::amountDebited
 * @return
 */
quint64 TxLogEntry::amountDebited() const
{
    return m_amountDebited;
}

/**
 * @brief TxLogEntry::setAmountDebited
 * @param v
 */
void TxLogEntry::setAmountDebited(quint64 v)
{
    m_amountDebited = v;
}

/**
 * @brief TxLogEntry::fee
 * @return
 */
QString TxLogEntry::fee() const
{
    return m_fee;
}

/**
 * @brief TxLogEntry::setFee
 * @param v
 */
void TxLogEntry::setFee(const QString &v)
{
    m_fee = v;
}

/**
 * @brief TxLogEntry::ttlCutoffHeight
 * @return
 */
quint64 TxLogEntry::ttlCutoffHeight() const
{
    return m_ttlCutoffHeight;
}

/**
 * @brief TxLogEntry::setTtlCutoffHeight
 * @param v
 */
void TxLogEntry::setTtlCutoffHeight(quint64 v)
{
    m_ttlCutoffHeight = v;
}

/**
 * @brief TxLogEntry::storedTx
 * @return
 */
QString TxLogEntry::storedTx() const
{
    return m_storedTx;
}

/**
 * @brief TxLogEntry::setStoredTx
 * @param v
 */
void TxLogEntry::setStoredTx(const QString &v)
{
    m_storedTx = v;
}

/**
 * @brief TxLogEntry::kernelExcess
 * @return
 */
QString TxLogEntry::kernelExcess() const
{
    return m_kernelExcess;
}

/**
 * @brief TxLogEntry::setKernelExcess
 * @param v
 */
void TxLogEntry::setKernelExcess(const QString &v)
{
    m_kernelExcess = v;
}

/**
 * @brief TxLogEntry::kernelLookupMinHeight
 * @return
 */
quint64 TxLogEntry::kernelLookupMinHeight() const
{
    return m_kernelLookupMinHeight;
}

/**
 * @brief TxLogEntry::setKernelLookupMinHeight
 * @param v
 */
void TxLogEntry::setKernelLookupMinHeight(quint64 v)
{
    m_kernelLookupMinHeight = v;
}

/**
 * @brief TxLogEntry::paymentProof
 * @return
 */
QString TxLogEntry::paymentProof() const
{
    return m_paymentProof;
}

/**
 * @brief TxLogEntry::setPaymentProof
 * @param v
 */
void TxLogEntry::setPaymentProof(const QString &v)
{
    m_paymentProof = v;
}

/**
 * @brief TxLogEntry::revertedAfterSeconds
 * @return
 */
qint64 TxLogEntry::revertedAfterSeconds() const
{
    return m_revertedAfterSeconds;
}

/**
 * @brief TxLogEntry::setRevertedAfterSeconds
 * @param v
 */
void TxLogEntry::setRevertedAfterSeconds(qint64 v)
{
    m_revertedAfterSeconds = v;
}

/**
 * @brief TxLogEntry::fromJson
 * @param json
 */
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

/**
 * @brief TxLogEntry::toJson
 * @return
 */
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
