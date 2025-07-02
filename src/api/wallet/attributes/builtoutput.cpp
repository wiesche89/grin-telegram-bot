#include "builtoutput.h"

/**
 * @brief BuiltOutput::BuiltOutput
 */
BuiltOutput::BuiltOutput()
{
}

/**
 * @brief BuiltOutput::blind
 * @return
 */
QString BuiltOutput::blind() const
{
    return m_blind;
}

/**
 * @brief BuiltOutput::setBlind
 * @param blind
 */
void BuiltOutput::setBlind(const QString &blind)
{
    m_blind = blind;
}

/**
 * @brief BuiltOutput::keyId
 * @return
 */
QString BuiltOutput::keyId() const
{
    return m_keyId;
}

/**
 * @brief BuiltOutput::setKeyId
 * @param keyId
 */
void BuiltOutput::setKeyId(const QString &keyId)
{
    m_keyId = keyId;
}

/**
 * @brief BuiltOutput::commit
 * @return
 */
QString BuiltOutput::commit() const
{
    return m_commit;
}

/**
 * @brief BuiltOutput::setCommit
 * @param commit
 */
void BuiltOutput::setCommit(const QString &commit)
{
    m_commit = commit;
}

/**
 * @brief BuiltOutput::features
 * @return
 */
QString BuiltOutput::features() const
{
    return m_features;
}

/**
 * @brief BuiltOutput::setFeatures
 * @param features
 */
void BuiltOutput::setFeatures(const QString &features)
{
    m_features = features;
}

/**
 * @brief BuiltOutput::proof
 * @return
 */
QString BuiltOutput::proof() const
{
    return m_proof;
}

/**
 * @brief BuiltOutput::setProof
 * @param proof
 */
void BuiltOutput::setProof(const QString &proof)
{
    m_proof = proof;
}

/**
 * @brief BuiltOutput::fromJson
 * @param rootObj
 * @return
 */
BuiltOutput BuiltOutput::fromJson(const QJsonObject &rootObj)
{
    BuiltOutput out;

    out.setBlind(rootObj.value("blind").toString());
    out.setKeyId(rootObj.value("key_id").toString());

    QJsonValue outputValue = rootObj.value("output");
    if (!outputValue.isObject()) {
        return out;
    }

    QJsonObject outputObj = outputValue.toObject();
    out.setCommit(outputObj.value("commit").toString());
    out.setFeatures(outputObj.value("features").toString());
    out.setProof(outputObj.value("proof").toString());

    return out;
}

/**
 * @brief BuiltOutput::toJson
 * @return
 */
QJsonObject BuiltOutput::toJson() const
{
    QJsonObject outputObj;
    outputObj["commit"] = m_commit;
    outputObj["features"] = m_features;
    outputObj["proof"] = m_proof;

    QJsonObject rootObj;
    rootObj["blind"] = m_blind;
    rootObj["key_id"] = m_keyId;
    rootObj["output"] = outputObj;

    return rootObj;
}
