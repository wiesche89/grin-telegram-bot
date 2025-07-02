#include "peeraddr.h"

/**
 * @brief PeerAddr::PeerAddr
 */
PeerAddr::PeerAddr()
{
}

/**
 * @brief PeerAddr::PeerAddr
 * @param obj
 */
PeerAddr::PeerAddr(const QJsonObject &obj) :
    m_obj(obj)
{
}

/**
 * @brief PeerAddr::toJson
 * @return
 */
QJsonObject PeerAddr::toJson() const
{
    return m_obj;
}

/**
 * @brief PeerAddr::fromJson
 * @param obj
 * @return
 */
PeerAddr PeerAddr::fromJson(const QJsonObject &obj)
{
    return PeerAddr(obj);
}
