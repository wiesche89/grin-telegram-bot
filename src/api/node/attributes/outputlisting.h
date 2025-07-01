#ifndef OUTPUTLISTING_H
#define OUTPUTLISTING_H

#include <QList>
#include <cstdint>
#include <QJsonObject>
#include <QJsonArray>
#include "outputprintable.h"

class OutputListing
{
public:
    OutputListing();

    uint64_t highestIndex() const;
    void setHighestIndex(uint64_t index);

    uint64_t lastRetrievedIndex() const;
    void setLastRetrievedIndex(uint64_t index);

    const QList<OutputPrintable> &outputs() const;
    void setOutputs(const QList<OutputPrintable> &outputs);
    void addOutput(const OutputPrintable &output);

    // JSON serialization
    static OutputListing fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    uint64_t m_highestIndex;
    uint64_t m_lastRetrievedIndex;
    QList<OutputPrintable> m_outputs;
};

#endif // OUTPUTLISTING_H
