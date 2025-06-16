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

// https://docs.rs/grin_api/latest/grin_api/foreign_rpc/trait.ForeignRpc.html
class NodeForeignApi : public QObject
{
    Q_OBJECT

public:
    NodeForeignApi(QString apiUrl, QString apiKey);

    QJsonObject getBlock();
    QJsonObject getBlocks();
    QJsonObject getHeader();
    QJsonObject getKernel();
    QJsonObject getOutputs();
    QJsonObject getPmmrIndices();
    QJsonObject getPoolSize();
    QJsonObject getStempoolSize();
    QJsonObject getTip();
    QJsonObject getUnconfirmedTransactions();
    QJsonObject getUnspentOutputs();
    QJsonObject getVersion();
    QJsonObject pushTransaction();

private:
    QJsonObject post(const QString &method, const QJsonObject &params);

    QString m_apiUrl;
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;
};

#endif // NODEFOREIGNAPI_H
