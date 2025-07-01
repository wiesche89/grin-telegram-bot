#include "peerdata.h"

static QString stateToString(PeerData::State state)
{
    switch (state) {
    case PeerData::Healthy:
        return "Healthy";
    case PeerData::Banned:
        return "Banned";
    case PeerData::Defunct:
        return "Defunct";
    default:
        return "Unknown";
    }
}

static PeerData::State stringToState(const QString &str)
{
    if (str == "Healthy") {
        return PeerData::Healthy;
    }
    if (str == "Banned") {
        return PeerData::Banned;
    }
    if (str == "Defunct") {
        return PeerData::Defunct;
    }
    return PeerData::Healthy; // Default fallback
}

// Hilfsfunktionen für ReasonForBan Enum <-> QString
static QString reasonForBanToString(PeerData::ReasonForBan reason)
{
    switch (reason) {
    case PeerData::None:
        return "None";
    case PeerData::BadBlock:
        return "BadBlock";
    case PeerData::BadCompactBlock:
        return "BadCompactBlock";
    case PeerData::BadBlockHeader:
        return "BadBlockHeader";
    case PeerData::BadTxHashSet:
        return "BadTxHashSet";
    case PeerData::ManualBan:
        return "ManualBan";
    case PeerData::FraudHeight:
        return "FraudHeight";
    case PeerData::BadHandshake:
        return "BadHandshake";
    default:
        return "Unknown";
    }
}

static PeerData::ReasonForBan stringToReasonForBan(const QString &str)
{
    if (str == "None") {
        return PeerData::None;
    }
    if (str == "BadBlock") {
        return PeerData::BadBlock;
    }
    if (str == "BadCompactBlock") {
        return PeerData::BadCompactBlock;
    }
    if (str == "BadBlockHeader") {
        return PeerData::BadBlockHeader;
    }
    if (str == "BadTxHashSet") {
        return PeerData::BadTxHashSet;
    }
    if (str == "ManualBan") {
        return PeerData::ManualBan;
    }
    if (str == "FraudHeight") {
        return PeerData::FraudHeight;
    }
    if (str == "BadHandshake") {
        return PeerData::BadHandshake;
    }
    return PeerData::None; // Default fallback
}

PeerData::PeerData() :
    flags(Healthy),
    lastBanned(0),
    banReason(None),
    lastConnected(0)
{
}

PeerAddr PeerData::getAddr() const
{
    return addr;
}

Capabilities PeerData::getCapabilities() const
{
    return capabilities;
}

QString PeerData::getUserAgent() const
{
    return userAgent;
}

PeerData::State PeerData::getFlags() const
{
    return flags;
}

qint64 PeerData::getLastBanned() const
{
    return lastBanned;
}

PeerData::ReasonForBan PeerData::getBanReason() const
{
    return banReason;
}

qint64 PeerData::getLastConnected() const
{
    return lastConnected;
}

void PeerData::setAddr(const PeerAddr &a)
{
    addr = a;
}

void PeerData::setCapabilities(const Capabilities &c)
{
    capabilities = c;
}

void PeerData::setUserAgent(const QString &ua)
{
    userAgent = ua;
}

void PeerData::setFlags(State f)
{
    flags = f;
}

void PeerData::setLastBanned(qint64 lb)
{
    lastBanned = lb;
}

void PeerData::setBanReason(ReasonForBan r)
{
    banReason = r;
}

void PeerData::setLastConnected(qint64 lc)
{
    lastConnected = lc;
}

PeerData PeerData::fromJson(const QJsonObject &obj)
{
    PeerData data;

    if (obj.contains("addr") && obj["addr"].isObject()) {
        data.addr = PeerAddr::fromJson(obj["addr"].toObject());
    }

    if (obj.contains("capabilities") && obj["capabilities"].isObject()) {
        data.capabilities = Capabilities::fromJson(obj["capabilities"].toObject());
    }

    if (obj.contains("user_agent")) {
        data.userAgent = obj["user_agent"].toString();
    }

    if (obj.contains("flags")) {
        data.flags = stringToState(obj["flags"].toString());
    }

    if (obj.contains("last_banned")) {
        data.lastBanned = obj["last_banned"].toVariant().toLongLong();
    }

    if (obj.contains("ban_reason")) {
        data.banReason = stringToReasonForBan(obj["ban_reason"].toString());
    }

    if (obj.contains("last_connected")) {
        data.lastConnected = obj["last_connected"].toVariant().toLongLong();
    }

    return data;
}

QJsonObject PeerData::toJson() const
{
    QJsonObject obj;
    obj["addr"] = addr.toJson();
    obj["capabilities"] = capabilities.toJson();
    obj["user_agent"] = userAgent;
    obj["flags"] = stateToString(flags);
    obj["last_banned"] = QString::number(lastBanned);
    obj["ban_reason"] = reasonForBanToString(banReason);
    obj["last_connected"] = QString::number(lastConnected);
    return obj;
}
