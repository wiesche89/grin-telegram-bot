#ifndef SUMMARYINFO_H
#define SUMMARYINFO_H

#include <QString>
#include <QJsonObject>

class SummaryInfo
{
public:

    SummaryInfo();

    static SummaryInfo fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

    qlonglong amountAwaitingConfirmation() const;
    void setAmountAwaitingConfirmation(qlonglong newAmountAwaitingConfirmation);

    qlonglong amountAwaitingFinalization() const;
    void setAmountAwaitingFinalization(qlonglong newAmountAwaitingFinalization);

    qlonglong amountCurrentlySpendable() const;
    void setAmountCurrentlySpendable(qlonglong newAmountCurrentlySpendable);

    qlonglong amountImmature() const;
    void setAmountImmature(qlonglong newAmountImmature);

    qlonglong amountLocked() const;
    void setAmountLocked(qlonglong newAmountLocked);

    qlonglong amountReverted() const;
    void setAmountReverted(qlonglong newAmountReverted);

    qint64 lastConfirmedHeight() const;
    void setLastConfirmedHeight(qint64 newLastConfirmedHeight);

    int minimumConfirmations() const;
    void setMinimumConfirmations(int newMinimumConfirmations);

    qlonglong total() const;
    void setTotal(qlonglong newTotal);

private:
    qlonglong m_amountAwaitingConfirmation;
    qlonglong m_amountAwaitingFinalization;
    qlonglong m_amountCurrentlySpendable;
    qlonglong m_amountImmature;
    qlonglong m_amountLocked;
    qlonglong m_amountReverted;
    qint64 m_lastConfirmedHeight;
    int m_minimumConfirmations;
    qlonglong m_total;
};

#endif // SUMMARYINFO_H
