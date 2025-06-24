#ifndef DONATE_H
#define DONATE_H

#include <QObject>

class Donate : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QString userId READ userId WRITE setUserId)
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString amount READ amount WRITE setAmount)
    Q_PROPERTY(QString date READ date WRITE setDate)

public:
    explicit Donate(QObject *parent = nullptr);
    Donate(int id, const QString &userId, const QString &username,
           const QString &amount, const QString &date, QObject *parent = nullptr);

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

#endif // DONATE_H
