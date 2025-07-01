#ifndef INPUT_H
#define INPUT_H

#include <QObject>
#include <QJsonObject>

#include "commitment.h"
#include "outputfeatures.h"

class Input
{
public:
    Input();
    Input(OutputFeatures::Feature features, Commitment commit);
    ~Input();

    OutputFeatures::Feature features() const;
    void setFeatures(OutputFeatures::Feature features);

    Commitment commit() const;
    void setCommit(Commitment commit);

    QJsonObject toJson() const;
    static Input fromJson(const QJsonObject &json);

private:
    OutputFeatures::Feature m_features;
    Commitment m_commit;
};

#endif // INPUT_H
