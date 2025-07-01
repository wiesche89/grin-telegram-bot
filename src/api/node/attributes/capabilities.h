#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include <QJsonObject>

class Capabilities
{
public:
    Capabilities();
    explicit Capabilities(const QJsonObject &obj);

    QJsonObject toJson() const;
    static Capabilities fromJson(const QJsonObject &obj);

private:
    QJsonObject m_obj;
};

#endif // CAPABILITIES_H
