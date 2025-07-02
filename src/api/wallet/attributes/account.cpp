#include "account.h"

/**
 * @brief Account::Account
 */
Account::Account() = default;

/**
 * @brief Account::Account
 * @param label
 * @param path
 */
Account::Account(const QString &label, const QString &path) :
    m_label(label),
    m_path(path)
{
}

/**
 * @brief Account::label
 * @return
 */
QString Account::label() const
{
    return m_label;
}

/**
 * @brief Account::path
 * @return
 */
QString Account::path() const
{
    return m_path;
}

/**
 * @brief Account::setLabel
 * @param label
 */
void Account::setLabel(const QString &label)
{
    m_label = label;
}

/**
 * @brief Account::setPath
 * @param path
 */
void Account::setPath(const QString &path)
{
    m_path = path;
}

/**
 * @brief Account::fromJson
 * @param obj
 * @return
 */
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

/**
 * @brief Account::toJson
 * @return
 */
QJsonObject Account::toJson() const
{
    QJsonObject obj;
    obj["label"] = m_label;
    obj["path"] = m_path;
    return obj;
}
