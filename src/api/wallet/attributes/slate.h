#ifndef SLATE_H
#define SLATE_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>

enum class SlateState {
    S1, // Standard: Sender has created initial Slate
    S2, // Standard: Recipient has added output, excess, partial sig
    S3, // Standard: Transaction complete, ready to post
    I1, // Invoice: Payee has created initial Slate
    I2, // Invoice: Payer has added inputs, change, partial sig
    I3, // Invoice: Transaction complete, ready to post
    Unknown
};

inline SlateState slateStateFromString(const QString &str)
{
    if (str == "S1") {
        return SlateState::S1;
    }
    if (str == "S2") {
        return SlateState::S2;
    }
    if (str == "S3") {
        return SlateState::S3;
    }
    if (str == "I1") {
        return SlateState::I1;
    }
    if (str == "I2") {
        return SlateState::I2;
    }
    if (str == "I3") {
        return SlateState::I3;
    }
    return SlateState::Unknown;
}

struct Signature {
    QString nonce;
    QString xs;

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["nonce"] = nonce;
        obj["xs"] = xs;
        return obj;
    }

    static Signature fromJson(const QJsonObject &obj)
    {
        Signature sig;
        sig.nonce = obj["nonce"].toString();
        sig.xs = obj["xs"].toString();
        return sig;
    }
};

class Slate
{
public:
    QString amt;
    QString fee;
    QString id;
    QList<Signature> sigs;
    QString sta;
    QString ver;

    QJsonObject toJson() const;
    static Slate fromJson(const QJsonObject &obj);
};

#endif // SLATE_H
