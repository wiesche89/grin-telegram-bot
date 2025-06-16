#ifndef SUMMARYINFO_H
#define SUMMARYINFO_H

#include <QString>
#include <QJsonObject>

class SummaryInfo {
public:
    qlonglong amountAwaitingConfirmation = 0;
    qlonglong amountAwaitingFinalization = 0;
    qlonglong amountCurrentlySpendable = 0;
    qlonglong amountImmature = 0;
    qlonglong amountLocked = 0;
    qlonglong amountReverted = 0;
    qint64 lastConfirmedHeight = 0;
    int minimumConfirmations = 0;
    qlonglong total = 0;

    static SummaryInfo fromJson(const QJsonObject& obj);
    QJsonObject toJson() const;
};

#endif // SUMMARYINFO_H
