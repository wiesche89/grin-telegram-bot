#include "builtoutput.h"

BuiltOutput::BuiltOutput()
{
}

QString BuiltOutput::blind() const
{
    return m_blind;
}

void BuiltOutput::setBlind(const QString &blind)
{
    m_blind = blind;
}

QString BuiltOutput::keyId() const
{
    return m_keyId;
}

void BuiltOutput::setKeyId(const QString &keyId)
{
    m_keyId = keyId;
}

QString BuiltOutput::commit() const
{
    return m_commit;
}

void BuiltOutput::setCommit(const QString &commit)
{
    m_commit = commit;
}

QString BuiltOutput::features() const
{
    return m_features;
}

void BuiltOutput::setFeatures(const QString &features)
{
    m_features = features;
}

QString BuiltOutput::proof() const
{
    return m_proof;
}

void BuiltOutput::setProof(const QString &proof)
{
    m_proof = proof;
}

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
