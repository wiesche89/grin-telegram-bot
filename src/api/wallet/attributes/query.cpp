#include "query.h"

/**
 * @brief Query::Query
 */
Query::Query() :
    m_minId(0),
    m_maxId(0)
{
}

/**
 * @brief Query::minId
 * @return
 */
int Query::minId() const
{
    return m_minId;
}

/**
 * @brief Query::setMinId
 * @param minId
 */
void Query::setMinId(int minId)
{
    m_minId = minId;
}

/**
 * @brief Query::maxId
 * @return
 */
int Query::maxId() const
{
    return m_maxId;
}

/**
 * @brief Query::setMaxId
 * @param maxId
 */
void Query::setMaxId(int maxId)
{
    m_maxId = maxId;
}

/**
 * @brief Query::minAmount
 * @return
 */
QString Query::minAmount() const
{
    return m_minAmount;
}

/**
 * @brief Query::setMinAmount
 * @param minAmount
 */
void Query::setMinAmount(const QString &minAmount)
{
    m_minAmount = minAmount;
}

/**
 * @brief Query::maxAmount
 * @return
 */
QString Query::maxAmount() const
{
    return m_maxAmount;
}

/**
 * @brief Query::setMaxAmount
 * @param maxAmount
 */
void Query::setMaxAmount(const QString &maxAmount)
{
    m_maxAmount = maxAmount;
}

/**
 * @brief Query::sortField
 * @return
 */
QString Query::sortField() const
{
    return m_sortField;
}

/**
 * @brief Query::setSortField
 * @param sortField
 */
void Query::setSortField(const QString &sortField)
{
    m_sortField = sortField;
}

/**
 * @brief Query::sortOrder
 * @return
 */
QString Query::sortOrder() const
{
    return m_sortOrder;
}

/**
 * @brief Query::setSortOrder
 * @param sortOrder
 */
void Query::setSortOrder(const QString &sortOrder)
{
    m_sortOrder = sortOrder;
}

/**
 * @brief Query::fromJson
 * @param json
 */
void Query::fromJson(const QJsonObject &json)
{
    m_minId = json.value("min_id").toInt();
    m_maxId = json.value("max_id").toInt();
    m_minAmount = json.value("min_amount").toString();
    m_maxAmount = json.value("max_amount").toString();
    m_sortField = json.value("sort_field").toString();
    m_sortOrder = json.value("sort_order").toString();
}

/**
 * @brief Query::toJson
 * @return
 */
QJsonObject Query::toJson() const
{
    QJsonObject json;
    json["min_id"] = m_minId;
    json["max_id"] = m_maxId;
    json["min_amount"] = m_minAmount;
    json["max_amount"] = m_maxAmount;
    json["sort_field"] = m_sortField;
    json["sort_order"] = m_sortOrder;
    return json;
}
