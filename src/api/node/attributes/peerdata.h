#ifndef PEERDATA_H
#define PEERDATA_H

#include <QString>
#include <QJsonObject>
#include <QVariant>

#include "peeraddr.h"
#include "capabilities.h"

class PeerData
{
public:

    enum State {
        Healthy = 0,
        Banned = 1,
        Defunct = 2,
    };

    enum ReasonForBan {
        None = 0,
        BadBlock = 1,
        BadCompactBlock = 2,
        BadBlockHeader = 3,
        BadTxHashSet = 4,
        ManualBan = 5,
        FraudHeight = 6,
        BadHandshake = 7,
    };

    PeerData();

    // Getters
    PeerAddr getAddr() const;
    Capabilities getCapabilities() const;
    QString getUserAgent() const;
    State getFlags() const;
    qint64 getLastBanned() const;
    ReasonForBan getBanReason() const;
    qint64 getLastConnected() const;

    // Setters
    void setAddr(const PeerAddr &addr);
    void setCapabilities(const Capabilities &capabilities);
    void setUserAgent(const QString &userAgent);
    void setFlags(State flags);
    void setLastBanned(qint64 lastBanned);
    void setBanReason(ReasonForBan reason);
    void setLastConnected(qint64 lastConnected);

    // Serialization
    static PeerData fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    PeerAddr m_addr;
    Capabilities m_capabilities;
    QString m_userAgent;
    State m_flags;
    qint64 m_lastBanned;
    ReasonForBan m_banReason;
    qint64 m_lastConnected;
};

#endif // PEERDATA_H
