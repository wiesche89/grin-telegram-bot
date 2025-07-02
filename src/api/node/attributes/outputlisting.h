#ifndef OUTPUTLISTING_H
#define OUTPUTLISTING_H

#include <QList>
#include <QJsonObject>
#include <QJsonArray>

#include "outputprintable.h"

class OutputListing
{
public:
    OutputListing();

    quint64 highestIndex() const;
    void setHighestIndex(quint64 index);

    quint64 lastRetrievedIndex() const;
    void setLastRetrievedIndex(quint64 index);

    const QList<OutputPrintable> &outputs() const;
    void setOutputs(const QList<OutputPrintable> &outputs);
    void addOutput(const OutputPrintable &output);

    // JSON serialization
    static OutputListing fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    quint64 m_highestIndex;
    quint64 m_lastRetrievedIndex;
    QList<OutputPrintable> m_outputs;
};

#endif // OUTPUTLISTING_H
