#include "blindingfactor.h"

BlindingFactor::BlindingFactor()
{
}

BlindingFactor::BlindingFactor(const QByteArray &data) :
    m_data(data)
{
}

QByteArray BlindingFactor::data() const
{
    return m_data;
}

void BlindingFactor::setData(const QByteArray &data)
{
    m_data = data;
}

QJsonObject BlindingFactor::toJson() const
{
    QJsonObject obj;
    obj["data"] = QString::fromUtf8(m_data.toHex());
    return obj;
}

BlindingFactor BlindingFactor::fromJson(const QJsonObject &json)
{
    BlindingFactor factor;
    if (json.contains("data")) {
        factor.setData(QByteArray::fromHex(json["data"].toString().toUtf8()));
    }
    return factor;
}
