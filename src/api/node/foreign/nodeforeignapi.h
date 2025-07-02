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
#include "result.h"
#include "jsonutil.h"

// https://docs.rs/grin_api/latest/grin_api/foreign_rpc/trait.ForeignRpc.html
// https://docs.grin.mw/grin-rfcs/text/0007-node-api-v2/
class NodeForeignApi : public QObject
{
    Q_OBJECT

public:
    NodeForeignApi(QString apiUrl, QString apiKey);

    Result<BlockPrintable> getBlock(int height, QString hash, QString commit);
    Result<BlockListing> getBlocks(int startHeight, int endHeight, int max, bool includeProof);
    Result<BlockHeaderPrintable> getHeader(int height, QString hash, QString commit);
    Result<LocatedTxKernel> getKernel(QString excess, int minHeight, int maxHeight);
    Result<QList<OutputPrintable> > getOutputs(QJsonArray commits, int startHeight, int endHeight, bool includeProof,
                                               bool includeMerkleProof);
    Result<OutputListing> getPmmrIndices(int startHeight, int endHeight);
    Result<int> getPoolSize();
    Result<int> getStempoolSize();
    Result<Tip> getTip();
    Result<QList<PoolEntry> > getUnconfirmedTransactions();
    Result<BlockListing> getUnspentOutputs(int startHeight, int endHeight, int max, bool includeProof);
    Result<NodeVersion> getVersion();
    Result<bool> pushTransaction(Transaction tx, bool fluff);

private:
    QJsonObject post(const QString &method, const QJsonArray &params);

    QString m_apiUrl;
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;
};

#endif // NODEFOREIGNAPI_H
