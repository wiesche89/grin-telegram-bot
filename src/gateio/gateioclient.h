#ifndef GATEIOCLIENT_H
#define GATEIOCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDebug>

class GateIoClient : public QObject
{
    Q_OBJECT
public:
    explicit GateIoClient(const QString &apiKey, const QString &apiSecret, QObject *parent = nullptr);

    // --- Spot Market ---
    void getCurrencies();
    void getCurrencyPairs();
    void getTicker(const QString &currencyPair);
    void getOrderBook(const QString &currencyPair, int limit = 10);
    void getTrades(const QString &currencyPair, int limit = 10);
    void getCandlesticks(const QString &currencyPair, const QString &interval = "1m", int limit = 100);

    // --- Spot Account ---
    void getSpotAccounts();
    void getSpotFee(const QString &currencyPair);

    // --- Spot Orders ---
    void placeSpotOrder(const QJsonObject &orderParams);
    void getSpotOrder(const QString &orderId);
    void cancelSpotOrder(const QString &orderId);
    void getOpenOrders(const QString &currencyPair = "");
    void getMyTrades(const QString &currencyPair = "", int limit = 100);
    void placeBatchOrders(const QJsonArray &orders);
    void cancelBatchOrders(const QJsonArray &ids);

signals:
    // Market
    void currenciesReceived(const QJsonArray &data);
    void currencyPairsReceived(const QJsonArray &data);
    void tickerReceived(const QJsonArray &data);
    void orderBookReceived(const QJsonObject &data);
    void tradesReceived(const QJsonArray &data);
    void candlesticksReceived(const QJsonArray &data);

    // Account
    void spotAccountsReceived(const QJsonArray &data);
    void spotFeeReceived(const QJsonObject &data);

    // Orders
    void orderPlaced(const QJsonObject &data);
    void orderReceived(const QJsonObject &data);
    void orderCanceled(const QJsonObject &data);
    void openOrdersReceived(const QJsonArray &data);
    void myTradesReceived(const QJsonArray &data);
    void batchOrdersPlaced(const QJsonArray &data);
    void batchOrdersCanceled(const QJsonArray &data);

    // Error
    void errorOccurred(const QString &errorString);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    /**
     * @brief GateIoClient::generateSignature
     * @param method
     * @param path
     * @param query
     * @param body
     * @param timestamp
     * @return
     */
    QString generateSignature(const QString &method, const QString &path, const QString &query, const QByteArray &body, const QString &timestamp);
    QNetworkRequest prepareRequest(const QString &method, const QString &path, const QString &query, const QByteArray &body);
    QString hashPayload(const QByteArray &body);

    QNetworkAccessManager m_manager;
    QString m_apiKey;
    QString m_apiSecret;
    const QString m_baseUrl = "https://api.gateio.ws/api/v4";
};

#endif // GATEIOCLIENT_H
