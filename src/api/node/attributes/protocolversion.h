#ifndef PROTOCOLVERSION_H
#define PROTOCOLVERSION_H

#include <QJsonObject>

class ProtocolVersion
{
public:
    ProtocolVersion();
    explicit ProtocolVersion(const QJsonObject &obj);

    QJsonObject toJson() const;
    static ProtocolVersion fromJson(const QJsonObject &obj);

private:
    QJsonObject m_obj;
};

#endif // PROTOCOLVERSION_H
