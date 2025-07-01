#include "query.h"

Query::Query() :
    m_minId(0),
    m_maxId(0)
{
}

int Query::minId() const
{
    return m_minId;
}

void Query::setMinId(int minId)
{
    m_minId = minId;
}

int Query::maxId() const
{
    return m_maxId;
}

void Query::setMaxId(int maxId)
{
    m_maxId = maxId;
}

QString Query::minAmount() const
{
    return m_minAmount;
}

void Query::setMinAmount(const QString &minAmount)
{
    m_minAmount = minAmount;
}

QString Query::maxAmount() const
{
    return m_maxAmount;
}

void Query::setMaxAmount(const QString &maxAmount)
{
    m_maxAmount = maxAmount;
}

QString Query::sortField() const
{
    return m_sortField;
}

void Query::setSortField(const QString &sortField)
{
    m_sortField = sortField;
}

QString Query::sortOrder() const
{
    return m_sortOrder;
}

void Query::setSortOrder(const QString &sortOrder)
{
    m_sortOrder = sortOrder;
}

void Query::fromJson(const QJsonObject &json)
{
    m_minId = json.value("min_id").toInt();
    m_maxId = json.value("max_id").toInt();
    m_minAmount = json.value("min_amount").toString();
    m_maxAmount = json.value("max_amount").toString();
    m_sortField = json.value("sort_field").toString();
    m_sortOrder = json.value("sort_order").toString();
}

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
