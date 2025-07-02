#include "outputidentifier.h"

/**
 * @brief OutputIdentifier::OutputIdentifier
 */
OutputIdentifier::OutputIdentifier() :
    m_features(OutputFeatures::Plain),
    m_commit()
{
}

/**
 * @brief OutputIdentifier::OutputIdentifier
 * @param features
 * @param commit
 */
OutputIdentifier::OutputIdentifier(OutputFeatures::Feature features, Commitment commit) :
    m_features(features),
    m_commit(commit)
{
}

/**
 * @brief OutputIdentifier::features
 * @return
 */
OutputFeatures::Feature OutputIdentifier::features() const
{
    return m_features;
}

/**
 * @brief OutputIdentifier::setFeatures
 * @param features
 */
void OutputIdentifier::setFeatures(OutputFeatures::Feature features)
{
    m_features = features;
}

/**
 * @brief OutputIdentifier::commit
 * @return
 */
Commitment OutputIdentifier::commit() const
{
    return m_commit;
}

/**
 * @brief OutputIdentifier::setCommit
 * @param commit
 */
void OutputIdentifier::setCommit(Commitment commit)
{
    m_commit = commit;
}

/**
 * @brief OutputIdentifier::toJson
 * @return
 */
QJsonObject OutputIdentifier::toJson() const
{
    QJsonObject json;
    json["features"] = static_cast<int>(m_features);
    json["commit"] = m_commit.toJson();
    return json;
}

/**
 * @brief OutputIdentifier::fromJson
 * @param json
 * @return
 */
OutputIdentifier OutputIdentifier::fromJson(const QJsonObject &json)
{
    OutputFeatures::Feature features = static_cast<OutputFeatures::Feature>(json.value("features").toInt());
    Commitment commit;
    if (json.contains("commit")) {
        commit.fromJson(json.value("commit").toObject());
    }
    return OutputIdentifier(features, commit);
}
