#ifndef SLATE_H
#define SLATE_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>

#include "signature.h"
#include "com.h"
#include "proof.h"
#include "error.h"

/**
 * @brief The SlateState enum
 */
enum class SlateState {
    S1, // Standard: Sender has created initial Slate
    S2, // Standard: Recipient has added output, excess, partial sig
    S3, // Standard: Transaction complete, ready to post
    I1, // Invoice: Payee has created initial Slate
    I2, // Invoice: Payer has added inputs, change, partial sig
    I3, // Invoice: Transaction complete, ready to post
    Unknown
};

class Slate
{
public:

    Slate();
    bool isValid();

    static SlateState slateStateFromString(const QString &str);

    QJsonObject toJson() const;
    static Slate fromJson(const QJsonObject &obj);

    QString amt() const;
    void setAmt(const QString &newAmt);

    QString fee() const;
    void setFee(const QString &newFee);

    QString id() const;
    void setId(const QString &newId);

    QList<Signature> sigs() const;
    void setSigs(const QList<Signature> &newSigs);

    QString sta() const;
    void setSta(const QString &newSta);

    QString ver() const;
    void setVer(const QString &newVer);

    QList<Com> coms() const;
    void setComs(const QList<Com> &newComs);

    QString off() const;
    void setOff(const QString &off);

    Proof proof() const;

    void setProof(const Proof &newProof);

private:
    QString m_amt;
    QString m_fee;
    QString m_id;
    QList<Signature> m_sigs;
    QList<Com> m_coms;
    Proof m_proof;
    QString m_sta;
    QString m_ver;
    QString m_off;
};

#endif // SLATE_H
