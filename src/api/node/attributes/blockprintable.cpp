#include "blockprintable.h"

/**
 * @brief BlockPrintable::BlockPrintable
 */
BlockPrintable::BlockPrintable() :
    m_header(),
    m_inputs(),
    m_outputs(),
    m_kernels()
{
}

/**
 * @brief BlockPrintable::header
 * @return
 */
BlockHeaderPrintable BlockPrintable::header() const
{
    return m_header;
}

/**
 * @brief BlockPrintable::inputs
 * @return
 */
QVector<QString> BlockPrintable::inputs() const
{
    return m_inputs;
}

/**
 * @brief BlockPrintable::outputs
 * @return
 */
QVector<OutputPrintable> BlockPrintable::outputs() const
{
    return m_outputs;
}

/**
 * @brief BlockPrintable::kernels
 * @return
 */
QVector<TxKernelPrintable> BlockPrintable::kernels() const
{
    return m_kernels;
}

/**
 * @brief BlockPrintable::setHeader
 * @param header
 */
void BlockPrintable::setHeader(const BlockHeaderPrintable &header)
{
    m_header = header;
}

/**
 * @brief BlockPrintable::setInputs
 * @param inputs
 */
void BlockPrintable::setInputs(const QVector<QString> &inputs)
{
    m_inputs = inputs;
}

/**
 * @brief BlockPrintable::setOutputs
 * @param outputs
 */
void BlockPrintable::setOutputs(const QVector<OutputPrintable> &outputs)
{
    m_outputs = outputs;
}

/**
 * @brief BlockPrintable::setKernels
 * @param kernels
 */
void BlockPrintable::setKernels(const QVector<TxKernelPrintable> &kernels)
{
    m_kernels = kernels;
}

/**
 * @brief BlockPrintable::fromJson
 * @param json
 */
void BlockPrintable::fromJson(const QJsonObject &json)
{
    if (json.contains("header") && json["header"].isObject()) {
        m_header.fromJson(json["header"].toObject());
    }

    m_inputs.clear();
    if (json.contains("inputs") && json["inputs"].isArray()) {
        QJsonArray arr = json["inputs"].toArray();
        for (const auto &v : arr) {
            m_inputs.append(v.toString());
        }
    }

    m_outputs.clear();
    if (json.contains("outputs") && json["outputs"].isArray()) {
        QJsonArray arr = json["outputs"].toArray();
        for (const auto &v : arr) {
            if (v.isObject()) {
                OutputPrintable op;
                op.fromJson(v.toObject());
                m_outputs.append(op);
            }
        }
    }

    m_kernels.clear();
    if (json.contains("kernels") && json["kernels"].isArray()) {
        QJsonArray arr = json["kernels"].toArray();
        for (const auto &v : arr) {
            if (v.isObject()) {
                TxKernelPrintable kernel;
                kernel.fromJson(v.toObject());
                m_kernels.append(kernel);
            }
        }
    }
}

/**
 * @brief BlockPrintable::toJson
 * @return
 */
QJsonObject BlockPrintable::toJson() const
{
    QJsonObject json;
    json["header"] = m_header.toJson();

    QJsonArray inputsArray;
    for (const auto &input : m_inputs) {
        inputsArray.append(input);
    }
    json["inputs"] = inputsArray;

    QJsonArray outputsArray;
    for (const auto &output : m_outputs) {
        outputsArray.append(output.toJson());
    }
    json["outputs"] = outputsArray;

    QJsonArray kernelsArray;
    for (const auto &kernel : m_kernels) {
        kernelsArray.append(kernel.toJson());
    }
    json["kernels"] = kernelsArray;

    return json;
}
