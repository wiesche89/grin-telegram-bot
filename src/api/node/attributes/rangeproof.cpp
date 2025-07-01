#include "rangeproof.h"

RangeProof::RangeProof() :
    m_plen(0)
{
}

QByteArray RangeProof::proof() const
{
    return m_proof;
}

void RangeProof::setProof(const QByteArray &proof)
{
    m_proof = proof;
}

int RangeProof::plen() const
{
    return m_plen;
}

void RangeProof::setPlen(int plen)
{
    m_plen = plen;
}

QJsonObject RangeProof::toJson() const
{
    QJsonObject json;
    json["proof"] = QString(m_proof.toHex());
    json["plen"] = m_plen;
    return json;
}

RangeProof RangeProof::fromJson(const QJsonObject &json)
{
    QByteArray proof = QByteArray::fromHex(json.value("proof").toString().toUtf8());
    int plen = json.value("plen").toInt();
    RangeProof rp;
    rp.setProof(proof);
    rp.setPlen(plen);
    return rp;
}
