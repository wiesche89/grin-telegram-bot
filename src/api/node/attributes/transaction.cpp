#include "transaction.h"

/**
 * @brief Transaction::fromJson
 * @param obj
 * @return
 */
Transaction Transaction::fromJson(const QJsonObject &obj)
{
    Transaction tx;
    if (obj.contains("offset") && obj["offset"].isObject()) {
        tx.m_offset = BlindingFactor::fromJson(obj["offset"].toObject());
    }
    if (obj.contains("body") && obj["body"].isObject()) {
        tx.m_body = TransactionBody::fromJson(obj["body"].toObject());
    }
    return tx;
}

/**
 * @brief Transaction::toJson
 * @return
 */
QJsonObject Transaction::toJson() const
{
    QJsonObject obj;
    obj["offset"] = m_offset.toJson();
    obj["body"] = m_body.toJson();
    return obj;
}

/**
 * @brief Transaction::offset
 * @return
 */
BlindingFactor Transaction::offset() const
{
    return m_offset;
}

/**
 * @brief Transaction::body
 * @return
 */
TransactionBody Transaction::body() const
{
    return m_body;
}

/**
 * @brief Transaction::setOffset
 * @param offset
 */
void Transaction::setOffset(const BlindingFactor &offset)
{
    m_offset = offset;
}

/**
 * @brief Transaction::setBody
 * @param body
 */
void Transaction::setBody(const TransactionBody &body)
{
    m_body = body;
}
