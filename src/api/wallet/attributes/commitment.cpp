#include "commitment.h"

/**
 * @brief Commitment::Commitment
 */
Commitment::Commitment() :
    m_data(33, 0)   // 33 Bytes
{
}

/**
 * @brief Commitment::Commitment
 * @param data
 */
Commitment::Commitment(const QByteArray &data)
{
    if (data.size() == 33) {
        m_data = data;
    } else {
        m_data = QByteArray(33, 0); // fallback
    }
}

/**
 * @brief Commitment::data
 * @return
 */
QByteArray Commitment::data() const
{
    return m_data;
}

/**
 * @brief Commitment::setData
 * @param data
 */
void Commitment::setData(const QByteArray &data)
{
    if (data.size() == 33) {
        m_data = data;
    }
}

/**
 * @brief Commitment::fromJson
 * Base64
 * @param json
 */
void Commitment::fromJson(const QJsonObject &json)
{
    if (json.contains("commitment") && json["commitment"].isString()) {
        QByteArray decoded = QByteArray::fromBase64(json["commitment"].toString().toUtf8());
        if (decoded.size() == 33) {
            m_data = decoded;
        } else {
            // Fallback/Error
            m_data = QByteArray(33, 0);
        }
    } else {
        m_data = QByteArray(33, 0);
    }
}

/**
 * @brief Commitment::toJson
 * Base64
 * @return
 */
QJsonObject Commitment::toJson() const
{
    QJsonObject json;
    json["commitment"] = QString::fromUtf8(m_data.toBase64());
    return json;
}
