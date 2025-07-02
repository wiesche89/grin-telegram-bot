#include "input.h"

/**
 * @brief Input::Input
 */
Input::Input() :
    m_features(OutputFeatures::Plain),
    m_commit()
{
}

/**
 * @brief Input::Input
 * @param features
 * @param commit
 */
Input::Input(OutputFeatures::Feature features, Commitment commit) :
    m_features(features),
    m_commit(commit)
{
}

/**
 * @brief Input::features
 * @return
 */
OutputFeatures::Feature Input::features() const
{
    return m_features;
}

/**
 * @brief Input::setFeatures
 * @param features
 */
void Input::setFeatures(OutputFeatures::Feature features)
{
    m_features = features;
}

/**
 * @brief Input::commit
 * @return
 */
Commitment Input::commit() const
{
    return m_commit;
}

/**
 * @brief Input::setCommit
 * @param commit
 */
void Input::setCommit(Commitment commit)
{
    m_commit = commit;
}

QJsonObject Input::toJson() const
{
    QJsonObject json;
    json["features"] = static_cast<int>(m_features);
    json["commit"] = m_commit.toJson();

    return json;
}

/**
 * @brief Input::fromJson
 * @param json
 * @return
 */
Input Input::fromJson(const QJsonObject &json)
{
    auto features = static_cast<OutputFeatures::Feature>(json.value("features").toInt());
    Commitment commit;
    if (json.contains("commit")) {
        commit.fromJson(json.value("commit").toObject());
    }
    return Input(features, commit);
}
