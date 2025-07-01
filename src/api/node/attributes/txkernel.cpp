#include "txkernel.h"
#include <QJsonObject>

TxKernel::TxKernel() :
    m_features(""),
    m_excess(""),
    m_excessSig("")
{
}

QString TxKernel::features() const
{
    return m_features;
}

QString TxKernel::excess() const
{
    return m_excess;
}

QString TxKernel::excessSig() const
{
    return m_excessSig;
}

void TxKernel::setFeatures(const QString &features)
{
    m_features = features;
}

void TxKernel::setExcess(const QString &excess)
{
    m_excess = excess;
}

void TxKernel::setExcessSig(const QString &excessSig)
{
    m_excessSig = excessSig;
}

void TxKernel::fromJson(const QJsonObject &json)
{
    if (json.contains("features") && json["features"].isString()) {
        m_features = json["features"].toString();
    }

    if (json.contains("excess") && json["excess"].isString()) {
        m_excess = json["excess"].toString();
    }

    if (json.contains("excess_sig") && json["excess_sig"].isString()) {
        m_excessSig = json["excess_sig"].toString();
    }
}

QJsonObject TxKernel::toJson() const
{
    QJsonObject json;
    json["features"] = m_features;
    json["excess"] = m_excess;
    json["excess_sig"] = m_excessSig;
    return json;
}
