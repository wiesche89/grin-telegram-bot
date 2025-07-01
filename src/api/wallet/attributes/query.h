#ifndef QUERY_H
#define QUERY_H

#include <QString>
#include <QJsonObject>

class Query
{
public:
    Query();

    int minId() const;
    void setMinId(int minId);

    int maxId() const;
    void setMaxId(int maxId);

    QString minAmount() const;
    void setMinAmount(const QString &minAmount);

    QString maxAmount() const;
    void setMaxAmount(const QString &maxAmount);

    QString sortField() const;
    void setSortField(const QString &sortField);

    QString sortOrder() const;
    void setSortOrder(const QString &sortOrder);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    int m_minId;
    int m_maxId;
    QString m_minAmount;
    QString m_maxAmount;
    QString m_sortField;
    QString m_sortOrder;
};

#endif // QUERY_H
