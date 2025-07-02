#include "proof.h"

/**
 * @brief Proof::Proof
 */
Proof::Proof() :
    m_raddr(""),
    m_saddr("")
{
}

/**
 * @brief Proof::isEmpty
 * @return
 */
bool Proof::isEmpty() const
{
    return m_raddr.isEmpty() || m_saddr.isEmpty();
}

/**
 * @brief Proof::raddr
 * @return
 */
QString Proof::raddr() const
{
    return m_raddr;
}

/**
 * @brief Proof::saddr
 * @return
 */
QString Proof::saddr() const
{
    return m_saddr;
}

/**
 * @brief Proof::setRaddr
 * @param raddr
 */
void Proof::setRaddr(const QString &raddr)
{
    m_raddr = raddr;
}

/**
 * @brief Proof::setSaddr
 * @param saddr
 */
void Proof::setSaddr(const QString &saddr)
{
    m_saddr = saddr;
}

/**
 * @brief Proof::fromJson
 * @param json
 * @return
 */
Proof Proof::fromJson(const QJsonObject &json)
{
    Proof proof;
    if (json.contains("raddr") && json["raddr"].isString()) {
        proof.setRaddr(json["raddr"].toString());
    }

    if (json.contains("saddr") && json["saddr"].isString()) {
        proof.setSaddr(json["saddr"].toString());
    }

    return proof;
}

/**
 * @brief Proof::toJson
 * @return
 */
QJsonObject Proof::toJson() const
{
    QJsonObject json;
    json["raddr"] = m_raddr;
    json["saddr"] = m_saddr;
    return json;
}
