#include "summaryinfo.h"

/**
 * @brief SummaryInfo::SummaryInfo
 */
SummaryInfo::SummaryInfo() :
    m_amountAwaitingConfirmation(0),
    m_amountAwaitingFinalization(0),
    m_amountCurrentlySpendable(0),
    m_amountImmature(0),
    m_amountLocked(0),
    m_amountReverted(0),
    m_lastConfirmedHeight(0),
    m_minimumConfirmations(0),
    m_total(0)
{
}

/**
 * @brief SummaryInfo::fromJson
 * @param obj
 * @return
 */
SummaryInfo SummaryInfo::fromJson(const QJsonObject &obj)
{
    SummaryInfo info;
    info.setAmountAwaitingConfirmation(obj["amount_awaiting_confirmation"].toString().toLongLong());
    info.setAmountAwaitingFinalization(obj["amount_awaiting_finalization"].toString().toLongLong());
    info.setAmountCurrentlySpendable(obj["amount_currently_spendable"].toString().toLongLong());
    info.setAmountImmature(obj["amount_immature"].toString().toLongLong());
    info.setAmountLocked(obj["amount_locked"].toString().toLongLong());
    info.setAmountReverted(obj["amount_reverted"].toString().toLongLong());
    info.setLastConfirmedHeight(obj["last_confirmed_height"].toString().toLongLong());
    info.setMinimumConfirmations(obj["minimum_confirmations"].toString().toInt());
    info.setTotal(obj["total"].toString().toLongLong());
    return info;
}

/**
 * @brief SummaryInfo::toJson
 * @return
 */
QJsonObject SummaryInfo::toJson() const
{
    QJsonObject obj;
    obj["amount_awaiting_confirmation"] = QString::number(m_amountAwaitingConfirmation);
    obj["amount_awaiting_finalization"] = QString::number(m_amountAwaitingFinalization);
    obj["amount_currently_spendable"] = QString::number(m_amountCurrentlySpendable);
    obj["amount_immature"] = QString::number(m_amountImmature);
    obj["amount_locked"] = QString::number(m_amountLocked);
    obj["amount_reverted"] = QString::number(m_amountReverted);
    obj["last_confirmed_height"] = QString::number(m_lastConfirmedHeight);
    obj["minimum_confirmations"] = QString::number(m_minimumConfirmations);
    obj["total"] = QString::number(m_total);
    return obj;
}

/**
 * @brief SummaryInfo::amountAwaitingConfirmation
 * @return
 */
qlonglong SummaryInfo::amountAwaitingConfirmation() const
{
    return m_amountAwaitingConfirmation;
}

/**
 * @brief SummaryInfo::setAmountAwaitingConfirmation
 * @param newAmountAwaitingConfirmation
 */
void SummaryInfo::setAmountAwaitingConfirmation(qlonglong newAmountAwaitingConfirmation)
{
    m_amountAwaitingConfirmation = newAmountAwaitingConfirmation;
}

/**
 * @brief SummaryInfo::amountAwaitingFinalization
 * @return
 */
qlonglong SummaryInfo::amountAwaitingFinalization() const
{
    return m_amountAwaitingFinalization;
}

/**
 * @brief SummaryInfo::setAmountAwaitingFinalization
 * @param newAmountAwaitingFinalization
 */
void SummaryInfo::setAmountAwaitingFinalization(qlonglong newAmountAwaitingFinalization)
{
    m_amountAwaitingFinalization = newAmountAwaitingFinalization;
}

/**
 * @brief SummaryInfo::amountCurrentlySpendable
 * @return
 */
qlonglong SummaryInfo::amountCurrentlySpendable() const
{
    return m_amountCurrentlySpendable;
}

/**
 * @brief SummaryInfo::setAmountCurrentlySpendable
 * @param newAmountCurrentlySpendable
 */
void SummaryInfo::setAmountCurrentlySpendable(qlonglong newAmountCurrentlySpendable)
{
    m_amountCurrentlySpendable = newAmountCurrentlySpendable;
}

/**
 * @brief SummaryInfo::amountImmature
 * @return
 */
qlonglong SummaryInfo::amountImmature() const
{
    return m_amountImmature;
}

/**
 * @brief SummaryInfo::setAmountImmature
 * @param newAmountImmature
 */
void SummaryInfo::setAmountImmature(qlonglong newAmountImmature)
{
    m_amountImmature = newAmountImmature;
}

/**
 * @brief SummaryInfo::amountLocked
 * @return
 */
qlonglong SummaryInfo::amountLocked() const
{
    return m_amountLocked;
}

/**
 * @brief SummaryInfo::setAmountLocked
 * @param newAmountLocked
 */
void SummaryInfo::setAmountLocked(qlonglong newAmountLocked)
{
    m_amountLocked = newAmountLocked;
}

/**
 * @brief SummaryInfo::amountReverted
 * @return
 */
qlonglong SummaryInfo::amountReverted() const
{
    return m_amountReverted;
}

/**
 * @brief SummaryInfo::setAmountReverted
 * @param newAmountReverted
 */
void SummaryInfo::setAmountReverted(qlonglong newAmountReverted)
{
    m_amountReverted = newAmountReverted;
}

/**
 * @brief SummaryInfo::lastConfirmedHeight
 * @return
 */
qint64 SummaryInfo::lastConfirmedHeight() const
{
    return m_lastConfirmedHeight;
}

/**
 * @brief SummaryInfo::setLastConfirmedHeight
 * @param newLastConfirmedHeight
 */
void SummaryInfo::setLastConfirmedHeight(qint64 newLastConfirmedHeight)
{
    m_lastConfirmedHeight = newLastConfirmedHeight;
}

/**
 * @brief SummaryInfo::minimumConfirmations
 * @return
 */
int SummaryInfo::minimumConfirmations() const
{
    return m_minimumConfirmations;
}

/**
 * @brief SummaryInfo::setMinimumConfirmations
 * @param newMinimumConfirmations
 */
void SummaryInfo::setMinimumConfirmations(int newMinimumConfirmations)
{
    m_minimumConfirmations = newMinimumConfirmations;
}

/**
 * @brief SummaryInfo::total
 * @return
 */
qlonglong SummaryInfo::total() const
{
    return m_total;
}

/**
 * @brief SummaryInfo::setTotal
 * @param newTotal
 */
void SummaryInfo::setTotal(qlonglong newTotal)
{
    m_total = newTotal;
}
