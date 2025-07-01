#ifndef POOLENTRY_H
#define POOLENTRY_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include "TxSource.h"
#include "Transaction.h"

class PoolEntry
{
public:
    PoolEntry() = default;

    static PoolEntry fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

    TxSource src() const;
    QDateTime txAt() const;
    Transaction tx() const;

    void setSrc(TxSource src);
    void setTxAt(const QDateTime &txAt);
    void setTx(const Transaction &tx);

private:
    TxSource m_src;
    QDateTime m_txAt;
    Transaction m_tx;
};

#endif // POOLENTRY_H
