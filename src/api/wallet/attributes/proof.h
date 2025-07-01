#ifndef PROOF_H
#define PROOF_H

#include <QString>
#include <QJsonObject>

class Proof
{
public:
    Proof();
    ~Proof() = default;

    bool isEmpty() const;

    // Getter
    QString raddr() const;
    QString saddr() const;

    // Setter
    void setRaddr(const QString &raddr);
    void setSaddr(const QString &saddr);

    // JSON
    static Proof fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QString m_raddr;
    QString m_saddr;
};

#endif // PROOF_H
