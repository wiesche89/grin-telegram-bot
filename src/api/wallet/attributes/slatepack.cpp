#include "slatepack.h"

/**
 * @brief Slatepack::Slatepack
 */
Slatepack::Slatepack() :
    m_mode(0)
{
}

/**
 * @brief Slatepack::mode
 * @return
 */
int Slatepack::mode() const
{
    return m_mode;
}

/**
 * @brief Slatepack::setMode
 * @param value
 */
void Slatepack::setMode(int value)
{
    m_mode = value;
}

/**
 * @brief Slatepack::payload
 * @return
 */
QString Slatepack::payload() const
{
    return m_payload;
}

/**
 * @brief Slatepack::setPayload
 * @param value
 */
void Slatepack::setPayload(const QString &value)
{
    m_payload = value;
}

/**
 * @brief Slatepack::sender
 * @return
 */
QString Slatepack::sender() const
{
    return m_sender;
}

/**
 * @brief Slatepack::setSender
 * @param value
 */
void Slatepack::setSender(const QString &value)
{
    m_sender = value;
}

/**
 * @brief Slatepack::slatepack
 * @return
 */
QString Slatepack::slatepack() const
{
    return m_slatepack;
}

/**
 * @brief Slatepack::setSlatepack
 * @param value
 */
void Slatepack::setSlatepack(const QString &value)
{
    m_slatepack = value;
}

/**
 * @brief Slatepack::fromJson
 * @param obj
 */
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

/**
 * @brief Slatepack::toJson
 * @return
 */
QJsonObject Slatepack::toJson() const
{
    QJsonObject obj;
    obj["mode"] = m_mode;
    obj["payload"] = m_payload;
    obj["sender"] = m_sender;
    obj["slatepack"] = m_slatepack;
    return obj;
}
