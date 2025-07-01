#ifndef OUTPUTCOMMITMAPPING_H
#define OUTPUTCOMMITMAPPING_H

#include <QJsonObject>
#include "outputdata.h"
#include "commitment.h"

class OutputCommitMapping
{
public:
    OutputCommitMapping();

    OutputData output() const;
    void setOutput(const OutputData &output);

    Commitment commit() const;
    void setCommit(const Commitment &commit);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    OutputData m_output;
    Commitment m_commit;
};

#endif // OUTPUTCOMMITMAPPING_H
