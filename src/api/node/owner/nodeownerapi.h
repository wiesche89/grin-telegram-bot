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

// https://docs.rs/grin_api/latest/grin_api/owner_rpc/trait.OwnerRpc.html
class NodeOwnerApi : public QObject
{
    Q_OBJECT

public:
    NodeOwnerApi(QString apiUrl, QString apiKey);

    QJsonObject banPeer();
    QJsonObject compactChain();
    QJsonObject getConnectedPeers();
    QJsonObject getPeers();
    QJsonObject getStatus();
    QJsonObject invalidateHeader();
    QJsonObject resetChainHead();
    QJsonObject unbanPeer();
    QJsonObject validateChain();

private:
    QJsonObject post(const QString &method, const QJsonObject &params);

    QString m_apiUrl;
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;
};

#endif // NODEOWNERAPI_H
