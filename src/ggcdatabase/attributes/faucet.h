#ifndef FAUCET_H
#define FAUCET_H

#include <QObject>

class Faucet
{
public:
    explicit Faucet();
    Faucet(int id, const QString &userId, const QString &username, const QString &amount, const QString &date);

    // Getter
    int id() const;
    QString userId() const;
    QString username() const;
    QString amount() const;
    QString date() const;

    // Setter
    void setId(int id);
    void setUserId(const QString &userId);
    void setUsername(const QString &username);
    void setAmount(const QString &amount);
    void setDate(const QString &date);

private:
    int m_id;
    QString m_userId;
    QString m_username;
    QString m_amount;
    QString m_date;
};

#endif // FAUCET_H
