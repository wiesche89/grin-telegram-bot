#include "outputidentifier.h"

OutputIdentifier::OutputIdentifier() :
    m_features(OutputFeatures::Plain),
    m_commit()
{
}

OutputIdentifier::OutputIdentifier(OutputFeatures::Feature features, Commitment commit) :
    m_features(features),
    m_commit(commit)
{
}

OutputFeatures::Feature OutputIdentifier::features() const
{
    return m_features;
}

void OutputIdentifier::setFeatures(OutputFeatures::Feature features)
{
    m_features = features;
}

Commitment OutputIdentifier::commit() const
{
    return m_commit;
}

void OutputIdentifier::setCommit(Commitment commit)
{
    m_commit = commit;
}

QJsonObject OutputIdentifier::toJson() const
{
    QJsonObject json;
    json["features"] = static_cast<int>(m_features);
    json["commit"] = m_commit.toJson();
    return json;
}

OutputIdentifier OutputIdentifier::fromJson(const QJsonObject &json)
{
    OutputFeatures::Feature features = static_cast<OutputFeatures::Feature>(json.value("features").toInt());
    Commitment commit;
    if (json.contains("commit")) {
        commit.fromJson(json.value("commit").toObject());
    }
    return OutputIdentifier(features, commit);
}
