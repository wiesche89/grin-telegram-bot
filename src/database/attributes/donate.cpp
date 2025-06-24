#include "donate.h"

Donate::Donate(QObject *parent)
    : QObject(parent), m_id(0), m_userId(""), m_username(""), m_amount(""), m_date("") {}

Donate::Donate(int id, const QString &userId, const QString &username,
               const QString &amount, const QString &date, QObject *parent)
    : QObject(parent), m_id(id), m_userId(userId), m_username(username),
      m_amount(amount), m_date(date) {}

int Donate::id() const {
    return m_id;
}

QString Donate::userId() const {
    return m_userId;
}

QString Donate::username() const {
    return m_username;
}

QString Donate::amount() const {
    return m_amount;
}

QString Donate::date() const {
    return m_date;
}

void Donate::setId(int id) {
    m_id = id;
}

void Donate::setUserId(const QString &userId) {
    m_userId = userId;
}

void Donate::setUsername(const QString &username) {
    m_username = username;
}

void Donate::setAmount(const QString &amount) {
    m_amount = amount;
}

void Donate::setDate(const QString &date) {
    m_date = date;
}
