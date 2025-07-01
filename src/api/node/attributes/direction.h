#ifndef DIRECTION_H
#define DIRECTION_H

#include <QJsonObject>

class Direction
{
public:
    Direction();
    explicit Direction(const QJsonObject &obj);

    QJsonObject toJson() const;
    static Direction fromJson(const QJsonObject &obj);

private:
    QJsonObject m_obj;
};

#endif // DIRECTION_H
