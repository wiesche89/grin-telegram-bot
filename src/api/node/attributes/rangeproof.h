#ifndef RANGEPROOF_H
#define RANGEPROOF_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>

class RangeProof
{
public:
    explicit RangeProof();

    QByteArray proof() const;
    void setProof(const QByteArray &proof);

    int plen() const;
    void setPlen(int plen);

    QJsonObject toJson() const;
    static RangeProof fromJson(const QJsonObject &json);

private:
    QByteArray m_proof; // expected size 5134 bytes
    int m_plen;
};

#endif // RANGEPROOF_H
