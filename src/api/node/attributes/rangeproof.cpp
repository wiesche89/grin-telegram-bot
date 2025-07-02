#include "rangeproof.h"

/**
 * @brief RangeProof::RangeProof
 */
RangeProof::RangeProof() :
    m_plen(0)
{
}

/**
 * @brief RangeProof::proof
 * @return
 */
QByteArray RangeProof::proof() const
{
    return m_proof;
}

/**
 * @brief RangeProof::setProof
 * @param proof
 */
void RangeProof::setProof(const QByteArray &proof)
{
    m_proof = proof;
}

/**
 * @brief RangeProof::plen
 * @return
 */
int RangeProof::plen() const
{
    return m_plen;
}

/**
 * @brief RangeProof::setPlen
 * @param plen
 */
void RangeProof::setPlen(int plen)
{
    m_plen = plen;
}

/**
 * @brief RangeProof::toJson
 * @return
 */
QJsonObject RangeProof::toJson() const
{
    QJsonObject json;
    json["proof"] = QString(m_proof.toHex());
    json["plen"] = m_plen;
    return json;
}

/**
 * @brief RangeProof::fromJson
 * @param json
 * @return
 */
RangeProof RangeProof::fromJson(const QJsonObject &json)
{
    QByteArray proof = QByteArray::fromHex(json.value("proof").toString().toUtf8());
    int plen = json.value("plen").toInt();
    RangeProof rp;
    rp.setProof(proof);
    rp.setPlen(plen);
    return rp;
}
