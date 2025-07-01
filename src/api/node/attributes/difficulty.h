#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <QJsonObject>

class Difficulty
{
public:
    Difficulty();
    explicit Difficulty(const QJsonObject &obj);

    QJsonObject toJson() const;
    static Difficulty fromJson(const QJsonObject &obj);

private:
    QJsonObject m_obj;
};

#endif // DIFFICULTY_H
