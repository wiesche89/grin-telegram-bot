#include "faucet.h"

/**
 * @brief Faucet::Faucet
 * @param parent
 */
Faucet::Faucet() :
    m_id(0),
    m_userId(""),
    m_username(""),
    m_amount(""),
    m_date("")
{
}

/**
 * @brief Faucet::Faucet
 * @param id
 * @param userId
 * @param username
 * @param amount
 * @param date
 * @param parent
 */
Faucet::Faucet(int id, const QString &userId, const QString &username, const QString &amount, const QString &date) :
    m_id(id),
    m_userId(userId),
    m_username(username),
    m_amount(amount),
    m_date(date)
{
}

/**
 * @brief Faucet::id
 * @return
 */
int Faucet::id() const
{
    return m_id;
}

/**
 * @brief Faucet::userId
 * @return
 */
QString Faucet::userId() const
{
    return m_userId;
}

/**
 * @brief Faucet::username
 * @return
 */
QString Faucet::username() const
{
    return m_username;
}

/**
 * @brief Faucet::amount
 * @return
 */
QString Faucet::amount() const
{
    return m_amount;
}

/**
 * @brief Faucet::date
 * @return
 */
QString Faucet::date() const
{
    return m_date;
}

/**
 * @brief Faucet::setId
 * @param id
 */
void Faucet::setId(int id)
{
    m_id = id;
}

/**
 * @brief Faucet::setUserId
 * @param userId
 */
void Faucet::setUserId(const QString &userId)
{
    m_userId = userId;
}

/**
 * @brief Faucet::setUsername
 * @param username
 */
void Faucet::setUsername(const QString &username)
{
    m_username = username;
}

/**
 * @brief Faucet::setAmount
 * @param amount
 */
void Faucet::setAmount(const QString &amount)
{
    m_amount = amount;
}

/**
 * @brief Faucet::setDate
 * @param date
 */
void Faucet::setDate(const QString &date)
{
    m_date = date;
}
