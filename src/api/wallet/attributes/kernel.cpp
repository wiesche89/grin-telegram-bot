#include "kernel.h"

/**
 * @brief Kernel::Kernel
 */
Kernel::Kernel() = default;

/**
 * @brief Kernel::Kernel
 * @param excess
 * @param excessSig
 * @param features
 */
Kernel::Kernel(const QString &excess, const QString &excessSig, const QString &features) :
    m_excess(excess),
    m_excessSig(excessSig),
    m_features(features)
{
}

/**
 * @brief Kernel::toJson
 * @return
 */
QJsonObject Kernel::toJson() const
{
    QJsonObject obj;
    obj["excess"] = m_excess;
    obj["excess_sig"] = m_excessSig;
    obj["features"] = m_features;
    return obj;
}

/**
 * @brief Kernel::fromJson
 * @param obj
 * @return
 */
Kernel Kernel::fromJson(const QJsonObject &obj)
{
    return Kernel(obj["excess"].toString(),
                  obj["excess_sig"].toString(),
                  obj["features"].toString());
}

/**
 * @brief Kernel::excess
 * @return
 */
QString Kernel::excess() const
{
    return m_excess;
}

/**
 * @brief Kernel::setExcess
 * @param excess
 */
void Kernel::setExcess(const QString &excess)
{
    m_excess = excess;
}

/**
 * @brief Kernel::excessSig
 * @return
 */
QString Kernel::excessSig() const
{
    return m_excessSig;
}

/**
 * @brief Kernel::setExcessSig
 * @param excessSig
 */
void Kernel::setExcessSig(const QString &excessSig)
{
    m_excessSig = excessSig;
}

/**
 * @brief Kernel::features
 * @return
 */
QString Kernel::features() const
{
    return m_features;
}

/**
 * @brief Kernel::setFeatures
 * @param features
 */
void Kernel::setFeatures(const QString &features)
{
    m_features = features;
}
