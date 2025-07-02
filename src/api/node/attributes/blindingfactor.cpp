#include "blindingfactor.h"

/**
 * @brief BlindingFactor::BlindingFactor
 */
BlindingFactor::BlindingFactor()
{
}

/**
 * @brief BlindingFactor::BlindingFactor
 * @param data
 */
BlindingFactor::BlindingFactor(const QByteArray &data) :
    m_data(data)
{
}

/**
 * @brief BlindingFactor::data
 * @return
 */
QByteArray BlindingFactor::data() const
{
    return m_data;
}

/**
 * @brief BlindingFactor::setData
 * @param data
 */
void BlindingFactor::setData(const QByteArray &data)
{
    m_data = data;
}

/**
 * @brief BlindingFactor::toJson
 * @return
 */
QJsonObject BlindingFactor::toJson() const
{
    QJsonObject obj;
    obj["data"] = QString::fromUtf8(m_data.toHex());
    return obj;
}

/**
 * @brief BlindingFactor::fromJson
 * @param json
 * @return
 */
BlindingFactor BlindingFactor::fromJson(const QJsonObject &json)
{
    BlindingFactor factor;
    if (json.contains("data")) {
        factor.setData(QByteArray::fromHex(json["data"].toString().toUtf8()));
    }
    return factor;
}
