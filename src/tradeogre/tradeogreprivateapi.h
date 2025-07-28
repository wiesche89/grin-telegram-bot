#ifndef TRADEOGREPRIVATEAPI_H
#define TRADEOGREPRIVATEAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QAuthenticator>
#include <QByteArray>
#include <QJsonDocument>


class TradeOgrePrivateApi : public QObject
{
    Q_OBJECT
public:
    TradeOgrePrivateApi(const QString &publicKey,
                        const QString &privateKey,
                        QObject *parent = nullptr);

    void submitBuyOrder(const QString &market, const QString &quantity, const QString &price, const QString &duration = "GTC");
    void submitSellOrder(const QString &market, const QString &quantity, const QString &price, const QString &duration = "GTC");
    void cancelOrder(const QString &uuid);
    void getOrders(const QString &market = QString());
    void getBalance(const QString &currency);
    void getBalances();
    void withdraw(const QString &currency, const QString &network, const QString &address, const QString &paymentid, const QString &quantity);

signals:
    void privateReply(const QString &endpoint, const QJsonDocument &json);
    void requestError(const QString &endpoint, const QString &errorString);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkRequest createRequest(const QString &endpoint);
    void post(const QString &endpoint, const QJsonObject &payload);

    QNetworkAccessManager m_nam;
    QString m_publicKey;
    QString m_privateKey;
};


#endif // TRADEOGREPRIVATEAPI_H
