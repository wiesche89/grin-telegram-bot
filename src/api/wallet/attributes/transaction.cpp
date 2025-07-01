#include "transaction.h"

/**
 * @brief Transaction::Transaction
 */
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

/**
 * @brief Transaction::amountCredited
 * @return
 */
qint64 Transaction::amountCredited() const
{
    return m_amountCredited;
}

/**
 * @brief Transaction::amountDebited
 * @return
 */
qint64 Transaction::amountDebited() const
{
    return m_amountDebited;
}

/**
 * @brief Transaction::confirmationTimestamp
 * @return
 */
QString Transaction::confirmationTimestamp() const
{
    return m_confirmationTs;
}

/**
 * @brief Transaction::isConfirmed
 * @return
 */
bool Transaction::isConfirmed() const
{
    return m_confirmed;
}

/**
 * @brief Transaction::creationTimestamp
 * @return
 */
QString Transaction::creationTimestamp() const
{
    return m_creationTs;
}

/**
 * @brief Transaction::fee
 * @return
 */
QString Transaction::fee() const
{
    return m_fee;
}

/**
 * @brief Transaction::id
 * @return
 */
int Transaction::id() const
{
    return m_id;
}

/**
 * @brief Transaction::kernelExcess
 * @return
 */
QString Transaction::kernelExcess() const
{
    return m_kernelExcess;
}

/**
 * @brief Transaction::kernelLookupMinHeight
 * @return
 */
int Transaction::kernelLookupMinHeight() const
{
    return m_kernelLookupMinHeight;
}

/**
 * @brief Transaction::numInputs
 * @return
 */
int Transaction::numInputs() const
{
    return m_numInputs;
}

/**
 * @brief Transaction::numOutputs
 * @return
 */
int Transaction::numOutputs() const
{
    return m_numOutputs;
}

/**
 * @brief Transaction::parentKeyId
 * @return
 */
QString Transaction::parentKeyId() const
{
    return m_parentKeyId;
}

/**
 * @brief Transaction::txSlateId
 * @return
 */
QString Transaction::txSlateId() const
{
    return m_txSlateId;
}

/**
 * @brief Transaction::txType
 * @return
 */
QString Transaction::txType() const
{
    return m_txType;
}

/**
 * @brief Transaction::setAmountCredited
 * @param value
 */
void Transaction::setAmountCredited(qint64 value)
{
    m_amountCredited = value;
}

/**
 * @brief Transaction::setAmountDebited
 * @param value
 */
void Transaction::setAmountDebited(qint64 value)
{
    m_amountDebited = value;
}

/**
 * @brief Transaction::setConfirmationTimestamp
 * @param value
 */
void Transaction::setConfirmationTimestamp(const QString &value)
{
    m_confirmationTs = value;
}

/**
 * @brief Transaction::setConfirmed
 * @param value
 */
void Transaction::setConfirmed(bool value)
{
    m_confirmed = value;
}

/**
 * @brief Transaction::setCreationTimestamp
 * @param value
 */
void Transaction::setCreationTimestamp(const QString &value)
{
    m_creationTs = value;
}

/**
 * @brief Transaction::setFee
 * @param value
 */
void Transaction::setFee(const QString &value)
{
    m_fee = value;
}

/**
 * @brief Transaction::setId
 * @param value
 */
void Transaction::setId(int value)
{
    m_id = value;
}

/**
 * @brief Transaction::setKernelExcess
 * @param value
 */
void Transaction::setKernelExcess(const QString &value)
{
    m_kernelExcess = value;
}

/**
 * @brief Transaction::setKernelLookupMinHeight
 * @param value
 */
void Transaction::setKernelLookupMinHeight(int value)
{
    m_kernelLookupMinHeight = value;
}

/**
 * @brief Transaction::setNumInputs
 * @param value
 */
void Transaction::setNumInputs(int value)
{
    m_numInputs = value;
}

/**
 * @brief Transaction::setNumOutputs
 * @param value
 */
void Transaction::setNumOutputs(int value)
{
    m_numOutputs = value;
}

/**
 * @brief Transaction::setParentKeyId
 * @param value
 */
void Transaction::setParentKeyId(const QString &value)
{
    m_parentKeyId = value;
}

/**
 * @brief Transaction::setTxSlateId
 * @param value
 */
void Transaction::setTxSlateId(const QString &value)
{
    m_txSlateId = value;
}

/**
 * @brief Transaction::setTxType
 * @param value
 */
void Transaction::setTxType(const QString &value)
{
    m_txType = value;
}

/**
 * @brief Transaction::toJson
 * @return
 */
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

/**
 * @brief Transaction::fromJson
 * @param obj
 * @return
 */
Transaction Transaction::fromJson(const QJsonObject &obj)
{
    Transaction tx;
    tx.setAmountCredited(obj["amount_credited"].toString().toLongLong());
    tx.setAmountDebited(obj["amount_debited"].toString().toLongLong());
    tx.setConfirmationTimestamp(obj["confirmation_ts"].toString());
    tx.setConfirmed(obj["confirmed"].toBool());
    tx.setCreationTimestamp(obj["creation_ts"].toString());
    tx.setFee(obj["fee"].toString());
    tx.setId(obj["id"].toInt());
    tx.setKernelExcess(obj["kernel_excess"].toString());
    tx.setKernelLookupMinHeight(obj["kernel_lookup_min_height"].toInt());
    tx.setNumInputs(obj["num_inputs"].toInt());
    tx.setNumOutputs(obj["num_outputs"].toInt());
    tx.setParentKeyId(obj["parent_key_id"].toString());
    tx.setTxSlateId(obj["tx_slate_id"].toString());
    tx.setTxType(obj["tx_type"].toString());
    return tx;
}
