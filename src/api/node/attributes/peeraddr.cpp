#include "peeraddr.h"

PeerAddr::PeerAddr()
{
}

PeerAddr::PeerAddr(const QJsonObject &obj) : m_obj(obj)
{
}

QJsonObject PeerAddr::toJson() const
{
    return m_obj;
}

PeerAddr PeerAddr::fromJson(const QJsonObject &obj)
{
    return PeerAddr(obj);
}
