#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QJsonObject>

#include "transactionbody.h"
#include "blindingfactor.h"

class Transaction
{
public:
    Transaction() = default;

    static Transaction fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

    BlindingFactor offset() const;
    TransactionBody body() const;

    void setOffset(const BlindingFactor &offset);
    void setBody(const TransactionBody &body);

private:
    BlindingFactor m_offset;
    TransactionBody m_body;
};

#endif // TRANSACTION_H
