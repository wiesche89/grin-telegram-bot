#ifndef BLOCKLISTING_H
#define BLOCKLISTING_H

#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

#include "blockprintable.h"

class BlockListing
{
public:
    BlockListing();

    // Getter
    quint64 lastRetrievedHeight() const;
    QVector<BlockPrintable> blocks() const;

    // Setter
    void setLastRetrievedHeight(quint64 height);
    void setBlocks(const QVector<BlockPrintable> &blocks);

    // JSON-Parsing
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    quint64 m_lastRetrievedHeight;
    QVector<BlockPrintable> m_blocks;
};

#endif // BLOCKLISTING_H
