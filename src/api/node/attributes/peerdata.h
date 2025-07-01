#ifndef PEERDATA_H
#define PEERDATA_H

#include <QString>
#include <QJsonObject>
#include <QVariant>

#include "PeerAddr.h"
#include "Capabilities.h"

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
    PeerAddr addr;
    Capabilities capabilities;
    QString userAgent;
    State flags;
    qint64 lastBanned;
    ReasonForBan banReason;
    qint64 lastConnected;
};

#endif // PEERDATA_H
