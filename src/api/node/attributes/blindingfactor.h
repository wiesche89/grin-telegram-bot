#ifndef BLINDINGFACTOR_H
#define BLINDINGFACTOR_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonArray>

class BlindingFactor
{
public:
    BlindingFactor();
    explicit BlindingFactor(const QByteArray &data);

    QByteArray data() const;
    void setData(const QByteArray &data);

    QJsonObject toJson() const;
    static BlindingFactor fromJson(const QJsonObject &json);

private:
    QByteArray m_data;
};

#endif // BLINDINGFACTOR_H
