#include "txkernelprintable.h"

/**
 * @brief TxKernelPrintable::TxKernelPrintable
 */
TxKernelPrintable::TxKernelPrintable() :
    m_feeShift(0),
    m_fee(0),
    m_lockHeight(0)
{
}

/**
 * @brief TxKernelPrintable::features
 * @return
 */
QString TxKernelPrintable::features() const
{
    return m_features;
}

/**
 * @brief TxKernelPrintable::feeShift
 * @return
 */
quint64 TxKernelPrintable::feeShift() const
{
    return m_feeShift;
}

/**
 * @brief TxKernelPrintable::fee
 * @return
 */
quint64 TxKernelPrintable::fee() const
{
    return m_fee;
}

/**
 * @brief TxKernelPrintable::lockHeight
 * @return
 */
quint64 TxKernelPrintable::lockHeight() const
{
    return m_lockHeight;
}

/**
 * @brief TxKernelPrintable::excess
 * @return
 */
QString TxKernelPrintable::excess() const
{
    return m_excess;
}

/**
 * @brief TxKernelPrintable::excessSig
 * @return
 */
QString TxKernelPrintable::excessSig() const
{
    return m_excessSig;
}

/**
 * @brief TxKernelPrintable::setFeatures
 * @param features
 */
void TxKernelPrintable::setFeatures(const QString &features)
{
    m_features = features;
}

/**
 * @brief TxKernelPrintable::setFeeShift
 * @param feeShift
 */
void TxKernelPrintable::setFeeShift(quint64 feeShift)
{
    m_feeShift = feeShift;
}

/**
 * @brief TxKernelPrintable::setFee
 * @param fee
 */
void TxKernelPrintable::setFee(quint64 fee)
{
    m_fee = fee;
}

/**
 * @brief TxKernelPrintable::setLockHeight
 * @param lockHeight
 */
void TxKernelPrintable::setLockHeight(quint64 lockHeight)
{
    m_lockHeight = lockHeight;
}

/**
 * @brief TxKernelPrintable::setExcess
 * @param excess
 */
void TxKernelPrintable::setExcess(const QString &excess)
{
    m_excess = excess;
}

/**
 * @brief TxKernelPrintable::setExcessSig
 * @param excessSig
 */
void TxKernelPrintable::setExcessSig(const QString &excessSig)
{
    m_excessSig = excessSig;
}

/**
 * @brief TxKernelPrintable::fromJson
 * @param json
 */
void TxKernelPrintable::fromJson(const QJsonObject &json)
{
    m_features = json.value("features").toString();
    m_feeShift = static_cast<uint8_t>(json.value("fee_shift").toInt());
    m_fee = json.value("fee").toString().toULongLong();
    m_lockHeight = json.value("lock_height").toString().toULongLong();
    m_excess = json.value("excess").toString();
    m_excessSig = json.value("excess_sig").toString();
}

/**
 * @brief TxKernelPrintable::toJson
 * @return
 */
QJsonObject TxKernelPrintable::toJson() const
{
    QJsonObject json;
    json["features"] = m_features;
    json["fee_shift"] = static_cast<int>(m_feeShift);
    json["fee"] = QString::number(m_fee);
    json["lock_height"] = QString::number(m_lockHeight);
    json["excess"] = m_excess;
    json["excess_sig"] = m_excessSig;
    return json;
}
