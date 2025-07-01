#ifndef WALLETINFO_H
#define WALLETINFO_H

#include <QObject>
#include <QJsonObject>
#include <QVariant>

class WalletInfo
{
public:
    WalletInfo();

    // Getter
    quint64 lastConfirmedHeight() const;
    quint64 minimumConfirmations() const;
    quint64 total() const;
    quint64 amountAwaitingFinalization() const;
    quint64 amountAwaitingConfirmation() const;
    quint64 amountImmature() const;
    quint64 amountCurrentlySpendable() const;
    quint64 amountLocked() const;
    quint64 amountReverted() const;

    // Setter
    void setLastConfirmedHeight(quint64 value);
    void setMinimumConfirmations(quint64 value);
    void setTotal(quint64 value);
    void setAmountAwaitingFinalization(quint64 value);
    void setAmountAwaitingConfirmation(quint64 value);
    void setAmountImmature(quint64 value);
    void setAmountCurrentlySpendable(quint64 value);
    void setAmountLocked(quint64 value);
    void setAmountReverted(quint64 value);

    // JSON serialization
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    quint64 m_lastConfirmedHeight;
    quint64 m_minimumConfirmations;
    quint64 m_total;
    quint64 m_amountAwaitingFinalization;
    quint64 m_amountAwaitingConfirmation;
    quint64 m_amountImmature;
    quint64 m_amountCurrentlySpendable;
    quint64 m_amountLocked;
    quint64 m_amountReverted;
};

#endif // WALLETINFO_H
