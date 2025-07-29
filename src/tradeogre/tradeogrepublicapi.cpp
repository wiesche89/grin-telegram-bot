#include "tradeogrepublicapi.h"

/**
 * @brief Constructor for TradeOgrePublicApi
 * Connects the QNetworkAccessManager to the reply handler
 */
TradeOgrePublicApi::TradeOgrePublicApi(QObject *parent) : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &TradeOgrePublicApi::onReplyFinished);
}

/**
 * @brief Fetches the list of available markets from TradeOgre
 */
void TradeOgrePublicApi::getMarkets() {
    m_nam.get(QNetworkRequest(QUrl("https://tradeogre.com/api/v1/markets")));
}

/**
 * @brief Fetches the order book for the specified market
 * @param market Market name, e.g. "GRIN-BTC"
 */
void TradeOgrePublicApi::getOrderBook(const QString &market) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/orders/" + market));
    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("market", market); // Store market for later use in the response
}

/**
 * @brief Fetches the current ticker data for the specified market
 * @param market Market name, e.g. "GRIN-USDT"
 */
void TradeOgrePublicApi::getTicker(const QString &market) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/ticker/" + market));
    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("market", market); // Tag reply with market
}

/**
 * @brief Fetches recent trade history for the specified market
 * @param market Market name
 */
void TradeOgrePublicApi::getTradeHistory(const QString &market) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/history/" + market));
    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("market", market); // Tag reply with market
}

/**
 * @brief Fetches OHLC chart data for a market
 * @param interval Time interval (1m, 15m, 1h, 4h, 1d, 1w)
 * @param market Market name
 * @param timestamp UNIX timestamp of the last candle (UTC)
 */
void TradeOgrePublicApi::getChart(const QString &interval, const QString &market, qint64 timestamp) {
    QString url = QString("https://tradeogre.com/api/v1/chart/%1/%2/%3")
    .arg(interval, market)
        .arg(timestamp);
    QNetworkReply *reply = m_nam.get(QNetworkRequest(QUrl(url)));
    reply->setProperty("market", market); // Tag reply with market
}

/**
 * @brief Handles all network responses
 * Routes based on URL path and emits appropriate signal
 */
void TradeOgrePublicApi::onReplyFinished(QNetworkReply *reply) {
    const QUrl url = reply->url();
    const QByteArray data = reply->readAll();
    QString market = reply->property("market").toString();  // May be empty if not set

    // Check for network error
    if (reply->error() != QNetworkReply::NoError) {
        emit requestError(url.toString(), reply->errorString());
        reply->deleteLater();
        return;
    }

    // Try parsing JSON response
    QJsonParseError jerr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jerr);
    if (jerr.error != QJsonParseError::NoError) {
        emit requestError(url.toString(), jerr.errorString());
        reply->deleteLater();
        return;
    }

    // Dispatch to appropriate signal based on URL path
    const QString path = url.path();

    if (path.contains("/markets")) {
        emit marketsReceived(doc.array());
    } else if (path.contains("/orders/")) {
        emit orderBookReceived(market, doc.object());
    } else if (path.contains("/ticker/")) {
        emit tickerReceived(market, doc.object());
    } else if (path.contains("/history/")) {
        emit tradeHistoryReceived(market, doc.array());
    } else if (path.contains("/chart/")) {
        emit chartDataReceived(market, doc.array());
    }

    reply->deleteLater();
}
