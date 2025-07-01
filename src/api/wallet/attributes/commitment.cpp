#include "Commitment.h"
#include <QJsonValue>
#include <QJsonDocument>
#include <QDebug>

Commitment::Commitment() :
    m_data(33, 0)   // 33 Bytes
{
}

Commitment::Commitment(const QByteArray &data)
{
    if (data.size() == 33) {
        m_data = data;
    } else {
        m_data = QByteArray(33, 0); // fallback
    }
}

QByteArray Commitment::data() const
{
    return m_data;
}

void Commitment::setData(const QByteArray &data)
{
    if (data.size() == 33) {
        m_data = data;
    }
}

// JSON: Base64 "commitment"
void Commitment::fromJson(const QJsonObject &json)
{
    if (json.contains("commitment") && json["commitment"].isString()) {
        QByteArray decoded = QByteArray::fromBase64(json["commitment"].toString().toUtf8());
        if (decoded.size() == 33) {
            m_data = decoded;
        } else {
            // Fallback oder Fehlerbehandlung
            m_data = QByteArray(33, 0);
        }
    } else {
        m_data = QByteArray(33, 0);
    }
}

// JSON erzeugen mit Base64-kodiertem Feld "commitment"
QJsonObject Commitment::toJson() const
{
    QJsonObject json;
    json["commitment"] = QString::fromUtf8(m_data.toBase64());
    return json;
}
