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

// https://docs.rs/grin_api/latest/grin_api/owner_rpc/trait.OwnerRpc.html
class NodeOwnerApi : public QObject
{
    Q_OBJECT

public:
    NodeOwnerApi(QString apiUrl, QString apiKey);

    bool banPeer(QString peerAddr);
    bool compactChain();
    QList<PeerInfoDisplay> getConnectedPeers();
    QList<PeerData> getPeers(QString peerAddr);
    Status getStatus();

    QJsonObject invalidateHeader(); // No description
    QJsonObject resetChainHead();   // No description
    QJsonObject unbanPeer(QString peerAddr);
    QJsonObject validateChain(bool assumeValidRangeproofsKernels);

private:
    QJsonObject post(const QString &method, const QJsonArray &params);

    QString m_apiUrl;
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;
};

#endif // NODEOWNERAPI_H
