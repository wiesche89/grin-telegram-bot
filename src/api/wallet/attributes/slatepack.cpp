#include "slatepack.h"

Slatepack::Slatepack() :
    m_mode(0)
{
}

int Slatepack::mode() const
{
    return m_mode;
}

void Slatepack::setMode(int value)
{
    m_mode = value;
}

QString Slatepack::payload() const
{
    return m_payload;
}

void Slatepack::setPayload(const QString &value)
{
    m_payload = value;
}

QString Slatepack::sender() const
{
    return m_sender;
}

void Slatepack::setSender(const QString &value)
{
    m_sender = value;
}

QString Slatepack::slatepack() const
{
    return m_slatepack;
}

void Slatepack::setSlatepack(const QString &value)
{
    m_slatepack = value;
}

void Slatepack::fromJson(const QJsonObject &obj)
{
    if (obj.contains("mode")) {
        m_mode = obj.value("mode").toInt();
    }

    if (obj.contains("payload")) {
        m_payload = obj.value("payload").toString();
    }

    if (obj.contains("sender")) {
        m_sender = obj.value("sender").toString();
    }

    if (obj.contains("slatepack")) {
        m_slatepack = obj.value("slatepack").toString();
    }
}

QJsonObject Slatepack::toJson() const
{
    QJsonObject obj;
    obj["mode"] = m_mode;
    obj["payload"] = m_payload;
    obj["sender"] = m_sender;
    obj["slatepack"] = m_slatepack;
    return obj;
}
