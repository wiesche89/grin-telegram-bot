#ifndef PEERADDR_H
#define PEERADDR_H

#include <QJsonObject>

class PeerAddr
{
public:
    PeerAddr();
    explicit PeerAddr(const QJsonObject &obj);

    QJsonObject toJson() const;
    static PeerAddr fromJson(const QJsonObject &obj);

private:
    QJsonObject m_obj;
};

#endif // PEERADDR_H
