#include "OutputCommitMapping.h"

OutputCommitMapping::OutputCommitMapping()
{
}

OutputData OutputCommitMapping::output() const
{
    return m_output;
}

void OutputCommitMapping::setOutput(const OutputData &output)
{
    m_output = output;
}

Commitment OutputCommitMapping::commit() const
{
    return m_commit;
}

void OutputCommitMapping::setCommit(const Commitment &commit)
{
    m_commit = commit;
}

void OutputCommitMapping::fromJson(const QJsonObject &json)
{
    if (json.contains("output") && json["output"].isObject()) {
        m_output.fromJson(json["output"].toObject());
    }

    if (json.contains("commit") && json["commit"].isObject()) {
        m_commit.fromJson(json["commit"].toObject());
    }
}

QJsonObject OutputCommitMapping::toJson() const
{
    QJsonObject json;
    json["output"] = m_output.toJson();
    json["commit"] = m_commit.toJson();
    return json;
}
