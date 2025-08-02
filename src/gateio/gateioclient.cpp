#include "gateioclient.h"

/**
 * @brief GateIoClient::GateIoClient
 * @param apiKey
 * @param apiSecret
 * @param parent
 */
GateIoClient::GateIoClient(const QString &apiKey, const QString &apiSecret, QObject *parent)
    : QObject(parent), m_apiKey(apiKey), m_apiSecret(apiSecret)
{
    connect(&m_manager, &QNetworkAccessManager::finished, this, &GateIoClient::onReplyFinished);
}

/**
 * @brief GateIoClient::hashPayload
 * @param body
 * @return
 */
QString GateIoClient::hashPayload(const QByteArray &body)
{
    return QString(QCryptographicHash::hash(body, QCryptographicHash::Sha512).toHex());
}

/**
 * @brief GateIoClient::generateSignature
 * @param method
 * @param path
 * @param query
 * @param body
 * @param timestamp
 * @return
 */
QString GateIoClient::generateSignature(const QString &method, const QString &path, const QString &query, const QByteArray &body, const QString &timestamp)
{
    QString hashedPayload = hashPayload(body);
    QString message = QString("%1\n%2\n%3\n%4\n%5")
                          .arg(method)
                          .arg(path)
                          .arg(query)
                          .arg(hashedPayload)
                          .arg(timestamp);

    QByteArray hmac = QMessageAuthenticationCode::hash(message.toUtf8(), m_apiSecret.toUtf8(), QCryptographicHash::Sha512);
    return QString(hmac.toHex());
}

/**
 * @brief GateIoClient::prepareRequest
 * @param method
 * @param path
 * @param query
 * @param body
 * @return
 */
QNetworkRequest GateIoClient::prepareRequest(const QString &method, const QString &path, const QString &query, const QByteArray &body)
{
    QString timestamp = QString::number(QDateTime::currentSecsSinceEpoch());
    QString sign = generateSignature(method, path, query, body, timestamp);
    QString fullUrl = m_baseUrl + path;
    if (!query.isEmpty())
        fullUrl += "?" + query;

    QUrl url(fullUrl);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("KEY", m_apiKey.toUtf8());
    req.setRawHeader("Timestamp", timestamp.toUtf8());
    req.setRawHeader("SIGN", sign.toUtf8());
    return req;

}

/**
 * @brief GateIoClient::getCurrencies
 */
void GateIoClient::getCurrencies()
{
    m_manager.get(QNetworkRequest(QUrl(m_baseUrl + "/spot/currencies")));
}

/**
 * @brief GateIoClient::getCurrencyPairs
 */
void GateIoClient::getCurrencyPairs()
{
    m_manager.get(QNetworkRequest(QUrl(m_baseUrl + "/spot/currency_pairs")));
}

/**
 * @brief GateIoClient::getTicker
 * @param currencyPair
 */
void GateIoClient::getTicker(const QString &currencyPair)
{
    QString path = "/spot/tickers";
    QString query = "currency_pair=" + currencyPair;
    QNetworkRequest req(QUrl(m_baseUrl + path + "?" + query));
    m_manager.get(req);
}

/**
 * @brief GateIoClient::getOrderBook
 * @param currencyPair
 * @param limit
 */
void GateIoClient::getOrderBook(const QString &currencyPair, int limit)
{
    QString path = "/spot/order_book";
    QString query = QString("currency_pair=%1&limit=%2").arg(currencyPair).arg(limit);
    m_manager.get(QNetworkRequest(QUrl(m_baseUrl + path + "?" + query)));
}

/**
 * @brief GateIoClient::getTrades
 * @param currencyPair
 * @param limit
 */
void GateIoClient::getTrades(const QString &currencyPair, int limit)
{
    QString path = "/spot/trades";
    QString query = QString("currency_pair=%1&limit=%2").arg(currencyPair).arg(limit);
    m_manager.get(QNetworkRequest(QUrl(m_baseUrl + path + "?" + query)));
}

/**
 * @brief GateIoClient::getCandlesticks
 * @param currencyPair
 * @param interval
 * @param limit
 */
void GateIoClient::getCandlesticks(const QString &currencyPair, const QString &interval, int limit)
{
    QString path = "/spot/candlesticks";
    QString query = QString("currency_pair=%1&interval=%2&limit=%3").arg(currencyPair).arg(interval).arg(limit);
    m_manager.get(QNetworkRequest(QUrl(m_baseUrl + path + "?" + query)));
}

/**
 * @brief GateIoClient::getSpotAccounts
 */
void GateIoClient::getSpotAccounts()
{
    QString path = "/spot/accounts";
    QNetworkRequest req = prepareRequest("GET", path, "", QByteArray());
    m_manager.get(req);
}

/**
 * @brief GateIoClient::getSpotFee
 * @param currencyPair
 */
void GateIoClient::getSpotFee(const QString &currencyPair)
{
    QString path = "/spot/fee";
    QString query = QString("currency_pair=%1").arg(currencyPair);
    QNetworkRequest req = prepareRequest("GET", path, query, QByteArray());
    m_manager.get(req);
}

/**
 * @brief GateIoClient::placeSpotOrder
 * @param orderParams
 */
void GateIoClient::placeSpotOrder(const QJsonObject &orderParams)
{
    QString path = "/spot/orders";
    QByteArray body = QJsonDocument(orderParams).toJson(QJsonDocument::Compact);
    QNetworkRequest req = prepareRequest("POST", path, "", body);
    m_manager.post(req, body);
}

/**
 * @brief GateIoClient::getSpotOrder
 * @param orderId
 */
void GateIoClient::getSpotOrder(const QString &orderId)
{
    QString path = "/spot/orders/" + orderId;
    QNetworkRequest req = prepareRequest("GET", path, "", QByteArray());
    m_manager.get(req);
}

/**
 * @brief GateIoClient::cancelSpotOrder
 * @param orderId
 */
void GateIoClient::cancelSpotOrder(const QString &orderId)
{
    QString path = "/spot/orders/" + orderId;
    QNetworkRequest req = prepareRequest("DELETE", path, "", QByteArray());
    m_manager.sendCustomRequest(req, "DELETE");
}

/**
 * @brief GateIoClient::getOpenOrders
 * @param currencyPair
 */
void GateIoClient::getOpenOrders(const QString &currencyPair)
{
    QString path = "/spot/open_orders";
    QString query = currencyPair.isEmpty() ? "" : QString("currency_pair=%1").arg(currencyPair);
    QNetworkRequest req = prepareRequest("GET", path, query, QByteArray());
    m_manager.get(req);
}

/**
 * @brief GateIoClient::getMyTrades
 * @param currencyPair
 * @param limit
 */
void GateIoClient::getMyTrades(const QString &currencyPair, int limit)
{
    QString path = "/spot/my_trades";
    QString query;
    if (!currencyPair.isEmpty())
        query = QString("currency_pair=%1&limit=%2").arg(currencyPair).arg(limit);
    QNetworkRequest req = prepareRequest("GET", path, query, QByteArray());
    m_manager.get(req);
}

/**
 * @brief GateIoClient::placeBatchOrders
 * @param orders
 */
void GateIoClient::placeBatchOrders(const QJsonArray &orders)
{
    QString path = "/spot/batch_orders";
    QByteArray body = QJsonDocument(orders).toJson(QJsonDocument::Compact);
    QNetworkRequest req = prepareRequest("POST", path, "", body);
    m_manager.post(req, body);
}

/**
 * @brief GateIoClient::cancelBatchOrders
 * @param ids
 */
void GateIoClient::cancelBatchOrders(const QJsonArray &ids)
{
    QString path = "/spot/cancel_batch_orders";
    QByteArray body = QJsonDocument(ids).toJson(QJsonDocument::Compact);
    QNetworkRequest req = prepareRequest("POST", path, "", body);
    m_manager.post(req, body);
}

/**
 * @brief GateIoClient::onReplyFinished
 * @param reply
 */
void GateIoClient::onReplyFinished(QNetworkReply *reply)
{
    QByteArray response = reply->readAll();
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parseErr);

    if (parseErr.error != QJsonParseError::NoError) {
        emit errorOccurred("Invalid JSON: " + parseErr.errorString());
        reply->deleteLater();
        return;
    }

    QString path = reply->url().path();

    if (path.contains("/currencies")) emit currenciesReceived(doc.array());
    else if (path.contains("/currency_pairs")) emit currencyPairsReceived(doc.array());
    else if (path.contains("/tickers")) emit tickerReceived(doc.array());
    else if (path.contains("/order_book")) emit orderBookReceived(doc.object());
    else if (path.contains("/trades")) emit tradesReceived(doc.array());
    else if (path.contains("/candlesticks")) emit candlesticksReceived(doc.array());
    else if (path.contains("/accounts")) emit spotAccountsReceived(doc.array());
    else if (path.contains("/fee")) emit spotFeeReceived(doc.object());
    else if (path.contains("/open_orders")) emit openOrdersReceived(doc.array());
    else if (path.contains("/my_trades")) emit myTradesReceived(doc.array());
    else if (path.contains("/batch_orders") && reply->operation() == QNetworkAccessManager::PostOperation) emit batchOrdersPlaced(doc.array());
    else if (path.contains("/cancel_batch_orders")) emit batchOrdersCanceled(doc.array());
    else if (path.contains("/orders") && reply->operation() == QNetworkAccessManager::PostOperation) emit orderPlaced(doc.object());
    else if (path.contains("/orders") && reply->operation() == QNetworkAccessManager::GetOperation) emit orderReceived(doc.object());
    else if (path.contains("/orders") && reply->operation() == QNetworkAccessManager::DeleteOperation) emit orderCanceled(doc.object());
    else emit errorOccurred("Unknown API response");

    reply->deleteLater();
}
