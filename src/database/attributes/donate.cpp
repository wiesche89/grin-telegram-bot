#include "donate.h"

/**
 * @brief Donate::Donate
 * @param parent
 */
Donate::Donate() :
    m_id(0),
    m_userId(""),
    m_username(""),
    m_amount(""),
    m_date("")
{
}

/**
 * @brief Donate::Donate
 * @param id
 * @param userId
 * @param username
 * @param amount
 * @param date
 * @param parent
 */
Donate::Donate(int id, const QString &userId, const QString &username, const QString &amount, const QString &date) :
    m_id(id),
    m_userId(userId),
    m_username(username),
    m_amount(amount),
    m_date(date)
{
}

/**
 * @brief Donate::id
 * @return
 */
int Donate::id() const
{
    return m_id;
}

/**
 * @brief Donate::userId
 * @return
 */
QString Donate::userId() const
{
    return m_userId;
}

/**
 * @brief Donate::username
 * @return
 */
QString Donate::username() const
{
    return m_username;
}

/**
 * @brief Donate::amount
 * @return
 */
QString Donate::amount() const
{
    return m_amount;
}

/**
 * @brief Donate::date
 * @return
 */
QString Donate::date() const
{
    return m_date;
}

/**
 * @brief Donate::setId
 * @param id
 */
void Donate::setId(int id)
{
    m_id = id;
}

/**
 * @brief Donate::setUserId
 * @param userId
 */
void Donate::setUserId(const QString &userId)
{
    m_userId = userId;
}

/**
 * @brief Donate::setUsername
 * @param username
 */
void Donate::setUsername(const QString &username)
{
    m_username = username;
}

/**
 * @brief Donate::setAmount
 * @param amount
 */
void Donate::setAmount(const QString &amount)
{
    m_amount = amount;
}

/**
 * @brief Donate::setDate
 * @param date
 */
void Donate::setDate(const QString &date)
{
    m_date = date;
}
