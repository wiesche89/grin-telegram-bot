#ifndef PEERINFODISPLAY_H
#define PEERINFODISPLAY_H

#include <QString>
#include <QJsonObject>

#include "capabilities.h"
#include "protocolversion.h"
#include "peeraddr.h"
#include "direction.h"
#include "difficulty.h"

class PeerInfoDisplay
{
public:
    PeerInfoDisplay();

    // Getter
    Capabilities capabilities() const;
    QString userAgent() const;
    ProtocolVersion version() const;
    PeerAddr addr() const;
    Direction direction() const;
    Difficulty totalDifficulty() const;
    quint64 height() const;

    // Setter
    void setCapabilities(const Capabilities &capabilities);
    void setUserAgent(const QString &userAgent);
    void setVersion(const ProtocolVersion &version);
    void setAddr(const PeerAddr &addr);
    void setDirection(const Direction &direction);
    void setTotalDifficulty(const Difficulty &difficulty);
    void setHeight(quint64 height);

    // JSON
    QJsonObject toJson() const;
    static PeerInfoDisplay fromJson(const QJsonObject &obj);

private:
    Capabilities m_capabilities;
    QString m_userAgent;
    ProtocolVersion m_version;
    PeerAddr m_addr;
    Direction m_direction;
    Difficulty m_totalDifficulty;
    quint64 m_height;
};

#endif // PEERINFODISPLAY_H
