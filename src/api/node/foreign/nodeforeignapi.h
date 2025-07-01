#ifndef NODEFOREIGNAPI_H
#define NODEFOREIGNAPI_H

#include <QObject>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

#include "blockheaderprintable.h"
#include "blockprintable.h"
#include "blocklisting.h"
#include "locatedtxkernel.h"
#include "outputlisting.h"
#include "tip.h"
#include "poolentry.h"
#include "nodeversion.h"

// https://docs.rs/grin_api/latest/grin_api/foreign_rpc/trait.ForeignRpc.html
class NodeForeignApi : public QObject
{
    Q_OBJECT

public:
    NodeForeignApi(QString apiUrl, QString apiKey);

    BlockPrintable getBlock(int height, QString hash, QString commit);
    BlockListing getBlocks(int startHeight, int endHeight, int max, bool includeProof);
    BlockHeaderPrintable getHeader(int height, QString hash, QString commit);
    LocatedTxKernel getKernel(QString excess, int minHeight, int maxHeight);
    QList<OutputPrintable> getOutputs(QJsonArray commits, int startHeight, int endHeight, bool includeProof, bool includeMerkleProof);
    OutputListing getPmmrIndices(int startHeight, int endHeight);
    int getPoolSize();
    int getStempoolSize();
    Tip getTip();
    QList<PoolEntry> getUnconfirmedTransactions();
    BlockListing getUnspentOutputs(int startHeight, int endHeight, int max, bool includeProof);
    NodeVersion getVersion();
    bool pushTransaction(Transaction tx, bool fluff);

private:
    QJsonObject post(const QString &method, const QJsonArray &params);

    QString m_apiUrl;
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;
};

#endif // NODEFOREIGNAPI_H
