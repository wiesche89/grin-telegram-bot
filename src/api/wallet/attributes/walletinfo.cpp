#include "walletinfo.h"

/**
 * @brief WalletInfo::WalletInfo
 */
WalletInfo::WalletInfo() :
    m_lastConfirmedHeight(0),
    m_minimumConfirmations(0),
    m_total(0),
    m_amountAwaitingFinalization(0),
    m_amountAwaitingConfirmation(0),
    m_amountImmature(0),
    m_amountCurrentlySpendable(0),
    m_amountLocked(0),
    m_amountReverted(0)
{
}

/**
 * @brief WalletInfo::lastConfirmedHeight
 * @return
 */
quint64 WalletInfo::lastConfirmedHeight() const
{
    return m_lastConfirmedHeight;
}

/**
 * @brief WalletInfo::minimumConfirmations
 * @return
 */
quint64 WalletInfo::minimumConfirmations() const
{
    return m_minimumConfirmations;
}

/**
 * @brief WalletInfo::total
 * @return
 */
quint64 WalletInfo::total() const
{
    return m_total;
}

/**
 * @brief WalletInfo::amountAwaitingFinalization
 * @return
 */
quint64 WalletInfo::amountAwaitingFinalization() const
{
    return m_amountAwaitingFinalization;
}

/**
 * @brief WalletInfo::amountAwaitingConfirmation
 * @return
 */
quint64 WalletInfo::amountAwaitingConfirmation() const
{
    return m_amountAwaitingConfirmation;
}

/**
 * @brief WalletInfo::amountImmature
 * @return
 */
quint64 WalletInfo::amountImmature() const
{
    return m_amountImmature;
}

/**
 * @brief WalletInfo::amountCurrentlySpendable
 * @return
 */
quint64 WalletInfo::amountCurrentlySpendable() const
{
    return m_amountCurrentlySpendable;
}

/**
 * @brief WalletInfo::amountLocked
 * @return
 */
quint64 WalletInfo::amountLocked() const
{
    return m_amountLocked;
}

/**
 * @brief WalletInfo::amountReverted
 * @return
 */
quint64 WalletInfo::amountReverted() const
{
    return m_amountReverted;
}

/**
 * @brief WalletInfo::setLastConfirmedHeight
 * @param value
 */
void WalletInfo::setLastConfirmedHeight(quint64 value)
{
    m_lastConfirmedHeight = value;
}

/**
 * @brief WalletInfo::setMinimumConfirmations
 * @param value
 */
void WalletInfo::setMinimumConfirmations(quint64 value)
{
    m_minimumConfirmations = value;
}

/**
 * @brief WalletInfo::setTotal
 * @param value
 */
void WalletInfo::setTotal(quint64 value)
{
    m_total = value;
}

/**
 * @brief WalletInfo::setAmountAwaitingFinalization
 * @param value
 */
void WalletInfo::setAmountAwaitingFinalization(quint64 value)
{
    m_amountAwaitingFinalization = value;
}

/**
 * @brief WalletInfo::setAmountAwaitingConfirmation
 * @param value
 */
void WalletInfo::setAmountAwaitingConfirmation(quint64 value)
{
    m_amountAwaitingConfirmation = value;
}

/**
 * @brief WalletInfo::setAmountImmature
 * @param value
 */
void WalletInfo::setAmountImmature(quint64 value)
{
    m_amountImmature = value;
}

/**
 * @brief WalletInfo::setAmountCurrentlySpendable
 * @param value
 */
void WalletInfo::setAmountCurrentlySpendable(quint64 value)
{
    m_amountCurrentlySpendable = value;
}

/**
 * @brief WalletInfo::setAmountLocked
 * @param value
 */
void WalletInfo::setAmountLocked(quint64 value)
{
    m_amountLocked = value;
}

/**
 * @brief WalletInfo::setAmountReverted
 * @param value
 */
void WalletInfo::setAmountReverted(quint64 value)
{
    m_amountReverted = value;
}

/**
 * @brief WalletInfo::fromJson
 * @param json
 */
void WalletInfo::fromJson(const QJsonObject &json)
{
    m_lastConfirmedHeight = json.value("last_confirmed_height").toVariant().toULongLong();
    m_minimumConfirmations = json.value("minimum_confirmations").toVariant().toULongLong();
    m_total = json.value("total").toVariant().toULongLong();
    m_amountAwaitingFinalization = json.value("amount_awaiting_finalization").toVariant().toULongLong();
    m_amountAwaitingConfirmation = json.value("amount_awaiting_confirmation").toVariant().toULongLong();
    m_amountImmature = json.value("amount_immature").toVariant().toULongLong();
    m_amountCurrentlySpendable = json.value("amount_currently_spendable").toVariant().toULongLong();
    m_amountLocked = json.value("amount_locked").toVariant().toULongLong();
    m_amountReverted = json.value("amount_reverted").toVariant().toULongLong();
}

/**
 * @brief WalletInfo::toJson
 * @return
 */
QJsonObject WalletInfo::toJson() const
{
    QJsonObject json;
    json["last_confirmed_height"] = QString::number(m_lastConfirmedHeight);
    json["minimum_confirmations"] = QString::number(m_minimumConfirmations);
    json["total"] = QString::number(m_total);
    json["amount_awaiting_finalization"] = QString::number(m_amountAwaitingFinalization);
    json["amount_awaiting_confirmation"] = QString::number(m_amountAwaitingConfirmation);
    json["amount_immature"] = QString::number(m_amountImmature);
    json["amount_currently_spendable"] = QString::number(m_amountCurrentlySpendable);
    json["amount_locked"] = QString::number(m_amountLocked);
    json["amount_reverted"] = QString::number(m_amountReverted);
    return json;
}
