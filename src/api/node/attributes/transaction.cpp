#include "transaction.h"

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

QJsonObject Transaction::toJson() const
{
    QJsonObject obj;
    obj["offset"] = m_offset.toJson();
    obj["body"] = m_body.toJson();
    return obj;
}

BlindingFactor Transaction::offset() const
{
    return m_offset;
}

TransactionBody Transaction::body() const
{
    return m_body;
}

void Transaction::setOffset(const BlindingFactor &offset)
{
    m_offset = offset;
}

void Transaction::setBody(const TransactionBody &body)
{
    m_body = body;
}
