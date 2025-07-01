#include "output.h"

/**
 * @brief Output::Output
 */
Output::Output() = default;

/**
 * @brief Output::Output
 * @param commit
 * @param features
 * @param proof
 */
Output::Output(const QString &commit, const QString &features, const QString &proof) :
    m_commit(commit),
    m_features(features),
    m_proof(proof)
{
}

/**
 * @brief Output::toJson
 * @return
 */
QJsonObject Output::toJson() const
{
    QJsonObject obj;
    obj["commit"] = m_commit;
    obj["features"] = m_features;
    obj["proof"] = m_proof;
    return obj;
}

/**
 * @brief Output::fromJson
 * @param obj
 * @return
 */
Output Output::fromJson(const QJsonObject &obj)
{
    return Output(obj["commit"].toString(),
                  obj["features"].toString(),
                  obj["proof"].toString());
}

/**
 * @brief Output::commit
 * @return
 */
QString Output::commit() const
{
    return m_commit;
}

/**
 * @brief Output::setCommit
 * @param commit
 */
void Output::setCommit(const QString &commit)
{
    m_commit = commit;
}

/**
 * @brief Output::features
 * @return
 */
QString Output::features() const
{
    return m_features;
}

/**
 * @brief Output::setFeatures
 * @param features
 */
void Output::setFeatures(const QString &features)
{
    m_features = features;
}

/**
 * @brief Output::proof
 * @return
 */
QString Output::proof() const
{
    return m_proof;
}

/**
 * @brief Output::setProof
 * @param proof
 */
void Output::setProof(const QString &proof)
{
    m_proof = proof;
}
