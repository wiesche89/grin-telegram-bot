#include "outputlisting.h"

OutputListing::OutputListing() :
    m_highestIndex(0),
    m_lastRetrievedIndex(0)
{
}

uint64_t OutputListing::highestIndex() const
{
    return m_highestIndex;
}

void OutputListing::setHighestIndex(uint64_t index)
{
    m_highestIndex = index;
}

uint64_t OutputListing::lastRetrievedIndex() const
{
    return m_lastRetrievedIndex;
}

void OutputListing::setLastRetrievedIndex(uint64_t index)
{
    m_lastRetrievedIndex = index;
}

const QList<OutputPrintable> &OutputListing::outputs() const
{
    return m_outputs;
}

void OutputListing::setOutputs(const QList<OutputPrintable> &outputs)
{
    m_outputs = outputs;
}

void OutputListing::addOutput(const OutputPrintable &output)
{
    m_outputs.append(output);
}

OutputListing OutputListing::fromJson(const QJsonObject &json)
{
    OutputListing listing;

    if (json.contains("highest_index") && json["highest_index"].isDouble()) {
        listing.setHighestIndex(static_cast<uint64_t>(json["highest_index"].toDouble()));
    }

    if (json.contains("last_retrieved_index") && json["last_retrieved_index"].isDouble()) {
        listing.setLastRetrievedIndex(static_cast<uint64_t>(json["last_retrieved_index"].toDouble()));
    }

    if (json.contains("outputs") && json["outputs"].isArray()) {
        QJsonArray outputsArray = json["outputs"].toArray();
        QList<OutputPrintable> outputsList;
        for (const QJsonValue &val : outputsArray) {
            if (val.isObject()) {
                OutputPrintable output;
                output.fromJson(val.toObject());
                outputsList.append(output);
            }
        }
        listing.setOutputs(outputsList);
    }

    return listing;
}

QJsonObject OutputListing::toJson() const
{
    QJsonObject json;
    json["highest_index"] = static_cast<double>(m_highestIndex);
    json["last_retrieved_index"] = static_cast<double>(m_lastRetrievedIndex);

    QJsonArray outputsArray;
    for (const OutputPrintable &output : m_outputs) {
        outputsArray.append(output.toJson());
    }
    json["outputs"] = outputsArray;

    return json;
}
