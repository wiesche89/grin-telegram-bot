#include "Account.h"

Account::Account() = default;

Account::Account(const QString &label, const QString &path) :
    m_label(label),
    m_path(path)
{
}

QString Account::label() const
{
    return m_label;
}

QString Account::path() const
{
    return m_path;
}

void Account::setLabel(const QString &label)
{
    m_label = label;
}

void Account::setPath(const QString &path)
{
    m_path = path;
}

Account Account::fromJson(const QJsonObject &obj)
{
    Account account;
    if (obj.contains("label") && obj["label"].isString()) {
        account.setLabel(obj["label"].toString());
    }
    if (obj.contains("path") && obj["path"].isString()) {
        account.setPath(obj["path"].toString());
    }
    return account;
}

QJsonObject Account::toJson() const
{
    QJsonObject obj;
    obj["label"] = m_label;
    obj["path"] = m_path;
    return obj;
}
