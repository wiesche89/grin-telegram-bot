#include "signature.h"

/**
 * @brief Signature::Signature
 */
Signature::Signature() = default;

/**
 * @brief Signature::Signature
 * @param nonce
 * @param xs
 */
Signature::Signature(const QString &nonce, const QString &xs, const QString &part) :
    m_nonce(nonce),
    m_xs(xs),
    m_part(part)
{
}

/**
 * @brief Signature::toJson
 * @return
 */
QJsonObject Signature::toJson() const
{
    QJsonObject obj;
    obj["nonce"] = m_nonce;
    obj["xs"] = m_xs;

    if(!m_part.isEmpty())
    {
     obj["part"] = m_part;
    }

    return obj;
}

/**
 * @brief Signature::fromJson
 * @param obj
 * @return
 */
Signature Signature::fromJson(const QJsonObject &obj)
{

    Signature sig;
    sig.setNonce(obj["nonce"].toString());
    sig.setXs(obj["xs"].toString());
    sig.setPart(obj["part"].toString());


    return sig;
}

/**
 * @brief Signature::nonce
 * @return
 */
QString Signature::nonce() const
{
    return m_nonce;
}

/**
 * @brief Signature::setNonce
 * @param nonce
 */
void Signature::setNonce(const QString &nonce)
{
    m_nonce = nonce;
}

/**
 * @brief Signature::xs
 * @return
 */
QString Signature::xs() const
{
    return m_xs;
}

/**
 * @brief Signature::setXs
 * @param xs
 */
void Signature::setXs(const QString &xs)
{
    m_xs = xs;
}

QString Signature::part() const
{
    return m_part;
}

void Signature::setPart(const QString &part)
{
    m_part = part;
}
