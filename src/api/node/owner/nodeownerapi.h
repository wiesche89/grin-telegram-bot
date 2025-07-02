#ifndef NODEOWNERAPI_H
#define NODEOWNERAPI_H

#include <QUrl>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

#include "peerinfodisplay.h"
#include "peerdata.h"
#include "status.h"
#include "result.h"
#include "jsonutil.h"

// https://docs.rs/grin_api/latest/grin_api/owner_rpc/trait.OwnerRpc.html
class NodeOwnerApi : public QObject
{
    Q_OBJECT

public:
    NodeOwnerApi(QString apiUrl, QString apiKey);

    Result<bool> banPeer(QString peerAddr);
    Result<bool> compactChain();
    Result<QList<PeerInfoDisplay> > getConnectedPeers();
    Result<QList<PeerData> > getPeers(QString peerAddr);
    Result<Status> getStatus();
    Result<bool> unbanPeer(QString peerAddr);
    Result<bool> validateChain(bool assumeValidRangeproofsKernels);

private:
    QJsonObject post(const QString &method, const QJsonArray &params);

    QString m_apiUrl;
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;
};

#endif // NODEOWNERAPI_H
