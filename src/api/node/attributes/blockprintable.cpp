#include "blockprintable.h"

BlockPrintable::BlockPrintable() :
    m_header(),
    m_inputs(),
    m_outputs(),
    m_kernels()
{
}

// Getter Implementierung
BlockHeaderPrintable BlockPrintable::header() const
{
    return m_header;
}

QVector<QString> BlockPrintable::inputs() const
{
    return m_inputs;
}

QVector<OutputPrintable> BlockPrintable::outputs() const
{
    return m_outputs;
}

QVector<TxKernelPrintable> BlockPrintable::kernels() const
{
    return m_kernels;
}

// Setter Implementierung
void BlockPrintable::setHeader(const BlockHeaderPrintable &header)
{
    m_header = header;
}

void BlockPrintable::setInputs(const QVector<QString> &inputs)
{
    m_inputs = inputs;
}

void BlockPrintable::setOutputs(const QVector<OutputPrintable> &outputs)
{
    m_outputs = outputs;
}

void BlockPrintable::setKernels(const QVector<TxKernelPrintable> &kernels)
{
    m_kernels = kernels;
}

// JSON parsing
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

// JSON serialization
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
