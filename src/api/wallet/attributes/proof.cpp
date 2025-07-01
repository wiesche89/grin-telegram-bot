#include "proof.h"

Proof::Proof() :
    m_raddr(""),
    m_saddr("")
{
}

bool Proof::isEmpty() const
{
    return m_raddr.isEmpty() || m_saddr.isEmpty();
}

QString Proof::raddr() const
{
    return m_raddr;
}

QString Proof::saddr() const
{
    return m_saddr;
}

void Proof::setRaddr(const QString &raddr)
{
    m_raddr = raddr;
}

void Proof::setSaddr(const QString &saddr)
{
    m_saddr = saddr;
}

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

QJsonObject Proof::toJson() const
{
    QJsonObject json;
    json["raddr"] = m_raddr;
    json["saddr"] = m_saddr;
    return json;
}
