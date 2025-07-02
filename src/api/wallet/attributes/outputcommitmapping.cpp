#include "outputcommitmapping.h"

/**
 * @brief OutputCommitMapping::OutputCommitMapping
 */
OutputCommitMapping::OutputCommitMapping()
{
}

/**
 * @brief OutputCommitMapping::output
 * @return
 */
OutputData OutputCommitMapping::output() const
{
    return m_output;
}

/**
 * @brief OutputCommitMapping::setOutput
 * @param output
 */
void OutputCommitMapping::setOutput(const OutputData &output)
{
    m_output = output;
}

/**
 * @brief OutputCommitMapping::commit
 * @return
 */
Commitment OutputCommitMapping::commit() const
{
    return m_commit;
}

/**
 * @brief OutputCommitMapping::setCommit
 * @param commit
 */
void OutputCommitMapping::setCommit(const Commitment &commit)
{
    m_commit = commit;
}

/**
 * @brief OutputCommitMapping::fromJson
 * @param json
 */
void OutputCommitMapping::fromJson(const QJsonObject &json)
{
    if (json.contains("output") && json["output"].isObject()) {
        m_output.fromJson(json["output"].toObject());
    }

    if (json.contains("commit") && json["commit"].isObject()) {
        m_commit.fromJson(json["commit"].toObject());
    }
}

/**
 * @brief OutputCommitMapping::toJson
 * @return
 */
QJsonObject OutputCommitMapping::toJson() const
{
    QJsonObject json;
    json["output"] = m_output.toJson();
    json["commit"] = m_commit.toJson();
    return json;
}
