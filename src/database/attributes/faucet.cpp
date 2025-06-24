#include "faucet.h"

Faucet::Faucet(QObject *parent)
    : QObject(parent), m_id(0), m_userId(""), m_username(""), m_amount(""), m_date("") {}

Faucet::Faucet(int id, const QString &userId, const QString &username,
               const QString &amount, const QString &date, QObject *parent)
    : QObject(parent), m_id(id), m_userId(userId), m_username(username),
      m_amount(amount), m_date(date) {}

int Faucet::id() const {
    return m_id;
}

QString Faucet::userId() const {
    return m_userId;
}

QString Faucet::username() const {
    return m_username;
}

QString Faucet::amount() const {
    return m_amount;
}

QString Faucet::date() const {
    return m_date;
}

void Faucet::setId(int id) {
    m_id = id;
}

void Faucet::setUserId(const QString &userId) {
    m_userId = userId;
}

void Faucet::setUsername(const QString &username) {
    m_username = username;
}

void Faucet::setAmount(const QString &amount) {
    m_amount = amount;
}

void Faucet::setDate(const QString &date) {
    m_date = date;
}
