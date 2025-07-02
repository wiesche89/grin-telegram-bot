#ifndef COMMITMENT_H
#define COMMITMENT_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDebug>

class Commitment
{
public:
    Commitment();
    explicit Commitment(const QByteArray &data);

    QByteArray data() const;
    void setData(const QByteArray &data);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QByteArray m_data;  // 33 Bytes
};

#endif // COMMITMENT_H
