#ifndef OUTPUTIDENTIFIER_H
#define OUTPUTIDENTIFIER_H

#include <QObject>
#include <QJsonObject>

#include "commitment.h"
#include "outputfeatures.h"

class OutputIdentifier
{
public:
    explicit OutputIdentifier();
    OutputIdentifier(OutputFeatures::Feature features, Commitment commit);

    OutputFeatures::Feature features() const;
    void setFeatures(OutputFeatures::Feature features);

    Commitment commit() const;
    void setCommit(Commitment commit);

    QJsonObject toJson() const;
    static OutputIdentifier fromJson(const QJsonObject &json);

private:
    OutputFeatures::Feature m_features;
    Commitment m_commit;
};

#endif // OUTPUTIDENTIFIER_H
