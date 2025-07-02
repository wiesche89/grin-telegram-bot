#include "outputlisting.h"

/**
 * @brief OutputListing::OutputListing
 */
OutputListing::OutputListing() :
    m_highestIndex(0),
    m_lastRetrievedIndex(0)
{
}

/**
 * @brief OutputListing::highestIndex
 * @return
 */
quint64 OutputListing::highestIndex() const
{
    return m_highestIndex;
}

/**
 * @brief OutputListing::setHighestIndex
 * @param index
 */
void OutputListing::setHighestIndex(quint64 index)
{
    m_highestIndex = index;
}

/**
 * @brief OutputListing::lastRetrievedIndex
 * @return
 */
quint64 OutputListing::lastRetrievedIndex() const
{
    return m_lastRetrievedIndex;
}

/**
 * @brief OutputListing::setLastRetrievedIndex
 * @param index
 */
void OutputListing::setLastRetrievedIndex(quint64 index)
{
    m_lastRetrievedIndex = index;
}

/**
 * @brief OutputListing::outputs
 * @return
 */
const QList<OutputPrintable> &OutputListing::outputs() const
{
    return m_outputs;
}

/**
 * @brief OutputListing::setOutputs
 * @param outputs
 */
void OutputListing::setOutputs(const QList<OutputPrintable> &outputs)
{
    m_outputs = outputs;
}

/**
 * @brief OutputListing::addOutput
 * @param output
 */
void OutputListing::addOutput(const OutputPrintable &output)
{
    m_outputs.append(output);
}

/**
 * @brief OutputListing::fromJson
 * @param json
 * @return
 */
OutputListing OutputListing::fromJson(const QJsonObject &json)
{
    OutputListing listing;

    if (json.contains("highest_index") && json["highest_index"].isDouble()) {
        listing.setHighestIndex(static_cast<quint64>(json["highest_index"].toDouble()));
    }

    if (json.contains("last_retrieved_index") && json["last_retrieved_index"].isDouble()) {
        listing.setLastRetrievedIndex(static_cast<quint64>(json["last_retrieved_index"].toDouble()));
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

/**
 * @brief OutputListing::toJson
 * @return
 */
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
