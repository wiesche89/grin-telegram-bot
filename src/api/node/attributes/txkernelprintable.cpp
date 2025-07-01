#include "txkernelprintable.h"

TxKernelPrintable::TxKernelPrintable() :
    m_feeShift(0),
    m_fee(0),
    m_lockHeight(0)
{
}

// Getter Implementierungen
QString TxKernelPrintable::features() const
{
    return m_features;
}

uint8_t TxKernelPrintable::feeShift() const
{
    return m_feeShift;
}

quint64 TxKernelPrintable::fee() const
{
    return m_fee;
}

quint64 TxKernelPrintable::lockHeight() const
{
    return m_lockHeight;
}

QString TxKernelPrintable::excess() const
{
    return m_excess;
}

QString TxKernelPrintable::excessSig() const
{
    return m_excessSig;
}

// Setter Implementierungen
void TxKernelPrintable::setFeatures(const QString &features)
{
    m_features = features;
}

void TxKernelPrintable::setFeeShift(uint8_t feeShift)
{
    m_feeShift = feeShift;
}

void TxKernelPrintable::setFee(quint64 fee)
{
    m_fee = fee;
}

void TxKernelPrintable::setLockHeight(quint64 lockHeight)
{
    m_lockHeight = lockHeight;
}

void TxKernelPrintable::setExcess(const QString &excess)
{
    m_excess = excess;
}

void TxKernelPrintable::setExcessSig(const QString &excessSig)
{
    m_excessSig = excessSig;
}

// JSON parsing
void TxKernelPrintable::fromJson(const QJsonObject &json)
{
    m_features = json.value("features").toString();
    m_feeShift = static_cast<uint8_t>(json.value("fee_shift").toInt());
    m_fee = json.value("fee").toString().toULongLong();
    m_lockHeight = json.value("lock_height").toString().toULongLong();
    m_excess = json.value("excess").toString();
    m_excessSig = json.value("excess_sig").toString();
}

// JSON creation
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
