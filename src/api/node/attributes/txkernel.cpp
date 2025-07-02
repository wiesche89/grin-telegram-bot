#include "txkernel.h"

/**
 * @brief TxKernel::TxKernel
 */
TxKernel::TxKernel() :
    m_features(""),
    m_excess(""),
    m_excessSig("")
{
}

/**
 * @brief TxKernel::features
 * @return
 */
QString TxKernel::features() const
{
    return m_features;
}

/**
 * @brief TxKernel::excess
 * @return
 */
QString TxKernel::excess() const
{
    return m_excess;
}

/**
 * @brief TxKernel::excessSig
 * @return
 */
QString TxKernel::excessSig() const
{
    return m_excessSig;
}

/**
 * @brief TxKernel::setFeatures
 * @param features
 */
void TxKernel::setFeatures(const QString &features)
{
    m_features = features;
}

/**
 * @brief TxKernel::setExcess
 * @param excess
 */
void TxKernel::setExcess(const QString &excess)
{
    m_excess = excess;
}

/**
 * @brief TxKernel::setExcessSig
 * @param excessSig
 */
void TxKernel::setExcessSig(const QString &excessSig)
{
    m_excessSig = excessSig;
}

/**
 * @brief TxKernel::fromJson
 * @param json
 */
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

/**
 * @brief TxKernel::toJson
 * @return
 */
QJsonObject TxKernel::toJson() const
{
    QJsonObject json;
    json["features"] = m_features;
    json["excess"] = m_excess;
    json["excess_sig"] = m_excessSig;
    return json;
}
