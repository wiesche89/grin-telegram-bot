#include "slate.h"

/**
 * @brief Slate::slateStateFromString
 * @param str
 * @return
 */
Slate::Slate() :
    m_amt(QString()),
    m_fee(QString()),
    m_id(QString()),
    m_sigs(QList<Signature>()),
    m_coms(QList<Com>()),
    m_proof(Proof()),
    m_sta(QString()),
    m_ver(QString()),
    m_off(QString()),
    m_error(Error())
{
}

bool Slate::isValid()
{
    if (m_id.isEmpty()) {
        return false;
    }

    return true;
}

SlateState Slate::slateStateFromString(const QString &str)
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

/**
 * @brief Slate::toJson
 * @return
 */
QJsonObject Slate::toJson() const
{
    QJsonObject obj;
    if (!m_amt.isEmpty()) {
        obj["amt"] = m_amt;
    }

    if (!m_fee.isEmpty()) {
        obj["fee"] = m_fee;
    }
    obj["id"] = m_id;

    QJsonArray sigArray;
    for (const auto &sig : m_sigs) {
        sigArray.append(sig.toJson());
    }
    obj["sigs"] = sigArray;

    QJsonArray comArray;
    for (const auto &com : m_coms) {
        comArray.append(com.toJson());
    }
    if (!comArray.isEmpty()) {
        obj["coms"] = comArray;
    }

    if (!m_proof.isEmpty()) {
        obj["proof"] = m_proof.toJson();
    }

    obj["sta"] = m_sta;
    obj["ver"] = m_ver;

    if (!m_off.isEmpty()) {
        obj["off"] = m_off;
    }

    return obj;
}

/**
 * @brief Slate::fromJson
 * @param obj
 * @return
 */
Slate Slate::fromJson(const QJsonObject &obj)
{
    Slate slate;

    slate.setAmt(obj["amt"].toString());
    slate.setFee(obj["fee"].toString());
    slate.setId(obj["id"].toString());

    QJsonArray sigArray = obj["sigs"].toArray();
    QList<Signature> sigs;
    for (const auto &sigVal : sigArray) {
        sigs.append(Signature::fromJson(sigVal.toObject()));
    }
    slate.setSigs(sigs);

    QList<Com> coms;
    if (obj.contains("coms") && obj.value("coms").isArray()) {
        QJsonArray arr = obj.value("coms").toArray();
        for (const auto &v : arr) {
            if (v.isObject()) {
                coms.append(Com::fromJson(v.toObject()));
            }
        }
    }
    slate.setComs(coms);
    slate.setProof(Proof::fromJson(obj["proof"].toObject()));
    slate.setSta(obj["sta"].toString());
    slate.setVer(obj["ver"].toString());
    slate.setOff(obj["off"].toString());

    return slate;
}

/**
 * @brief Slate::amt
 * @return
 */
QString Slate::amt() const
{
    return m_amt;
}

/**
 * @brief Slate::setAmt
 * @param newAmt
 */
void Slate::setAmt(const QString &newAmt)
{
    m_amt = newAmt;
}

/**
 * @brief Slate::fee
 * @return
 */
QString Slate::fee() const
{
    return m_fee;
}

/**
 * @brief Slate::setFee
 * @param newFee
 */
void Slate::setFee(const QString &newFee)
{
    m_fee = newFee;
}

/**
 * @brief Slate::id
 * @return
 */
QString Slate::id() const
{
    return m_id;
}

/**
 * @brief Slate::setId
 * @param newId
 */
void Slate::setId(const QString &newId)
{
    m_id = newId;
}

/**
 * @brief Slate::sigs
 * @return
 */
QList<Signature> Slate::sigs() const
{
    return m_sigs;
}

/**
 * @brief Slate::setSigs
 * @param newSigs
 */
void Slate::setSigs(const QList<Signature> &newSigs)
{
    m_sigs = newSigs;
}

/**
 * @brief Slate::sta
 * @return
 */
QString Slate::sta() const
{
    return m_sta;
}

/**
 * @brief Slate::setSta
 * @param newSta
 */
void Slate::setSta(const QString &newSta)
{
    m_sta = newSta;
}

/**
 * @brief Slate::ver
 * @return
 */
QString Slate::ver() const
{
    return m_ver;
}

/**
 * @brief Slate::setVer
 * @param newVer
 */
void Slate::setVer(const QString &newVer)
{
    m_ver = newVer;
}

/**
 * @brief Slate::coms
 * @return
 */
QList<Com> Slate::coms() const
{
    return m_coms;
}

/**
 * @brief Slate::setComs
 * @param newComs
 */
void Slate::setComs(const QList<Com> &newComs)
{
    m_coms = newComs;
}

Error Slate::error() const
{
    return m_error;
}

void Slate::setError(const Error &error)
{
    m_error = error;
}

QString Slate::off() const
{
    return m_off;
}

void Slate::setOff(const QString &off)
{
    m_off = off;
}

Proof Slate::proof() const
{
    return m_proof;
}

void Slate::setProof(const Proof &newProof)
{
    m_proof = newProof;
}
