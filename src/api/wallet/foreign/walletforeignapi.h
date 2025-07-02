#ifndef WALLETFOREIGNAPI_H
#define WALLETFOREIGNAPI_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

#include "coinbase.h"
#include "version.h"
#include "slate.h"
#include "result.h"
#include "jsonutil.h"

// https://docs.rs/grin_wallet_api/5.3.3/grin_wallet_api/trait.ForeignRpc.html#tymethod.receive_tx
class WalletForeignApi : public QObject
{
    Q_OBJECT
public:
    WalletForeignApi(QString apiUrl);

    Result<Coinbase> buildCoinbase(int fees, int height, QString keyId);
    Result<Version> checkVersion();
    Result<Slate> finalizeTx(Slate slate);
    Result<Slate> receiveTx(Slate slate, QString destAcctName, QString dest);

private:
    QJsonObject post(const QString &method, const QJsonObject &params);

    QString m_apiUrl;
    QNetworkAccessManager *m_networkManager;
};

#endif // WALLETFOREIGNAPI_H
