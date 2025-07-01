#include "poolentry.h"

PoolEntry PoolEntry::fromJson(const QJsonObject &obj)
{
    PoolEntry entry;
    if (obj.contains("src") && obj["src"].isString()) {
        entry.m_src = txSourceFromString(obj["src"].toString());
    }
    if (obj.contains("tx_at") && obj["tx_at"].isString()) {
        entry.m_txAt = QDateTime::fromString(obj["tx_at"].toString(), Qt::ISODate);
    }
    if (obj.contains("tx") && obj["tx"].isObject()) {
        entry.m_tx = Transaction::fromJson(obj["tx"].toObject());
    }
    return entry;
}

QJsonObject PoolEntry::toJson() const
{
    QJsonObject obj;
    obj["src"] = txSourceToString(m_src);
    obj["tx_at"] = m_txAt.toString(Qt::ISODate);
    obj["tx"] = m_tx.toJson();
    return obj;
}

TxSource PoolEntry::src() const
{
    return m_src;
}

QDateTime PoolEntry::txAt() const
{
    return m_txAt;
}

Transaction PoolEntry::tx() const
{
    return m_tx;
}

void PoolEntry::setSrc(TxSource src)
{
    m_src = src;
}

void PoolEntry::setTxAt(const QDateTime &txAt)
{
    m_txAt = txAt;
}

void PoolEntry::setTx(const Transaction &tx)
{
    m_tx = tx;
}
