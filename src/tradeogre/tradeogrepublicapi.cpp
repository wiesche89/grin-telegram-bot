#include "tradeogrepublicapi.h"

TradeOgrePublicApi::TradeOgrePublicApi(QObject *parent) : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &TradeOgrePublicApi::onReplyFinished);
}

void TradeOgrePublicApi::getMarkets() {
    m_nam.get(QNetworkRequest(QUrl("https://tradeogre.com/api/v1/markets")));
}

void TradeOgrePublicApi::getOrderBook(const QString &market) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/orders/" + market));
    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("market", market);
}

void TradeOgrePublicApi::getTicker(const QString &market) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/ticker/" + market));
    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("market", market);
}

void TradeOgrePublicApi::getTradeHistory(const QString &market) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/history/" + market));
    QNetworkReply *reply = m_nam.get(req);
    reply->setProperty("market", market);
}

void TradeOgrePublicApi::getChart(const QString &interval, const QString &market, qint64 timestamp) {
    QString url = QString("https://tradeogre.com/api/v1/chart/%1/%2/%3").arg(interval, market).arg(timestamp);
    QNetworkReply *reply = m_nam.get(QNetworkRequest(QUrl(url)));
    reply->setProperty("market", market);
}

void TradeOgrePublicApi::onReplyFinished(QNetworkReply *reply) {
    const QUrl url = reply->url();
    const QByteArray data = reply->readAll();
    QString market = reply->property("market").toString();  // Falls gesetzt

    if (reply->error() != QNetworkReply::NoError) {
        emit requestError(url.toString(), reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonParseError jerr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jerr);
    if (jerr.error != QJsonParseError::NoError) {
        emit requestError(url.toString(), jerr.errorString());
        reply->deleteLater();
        return;
    }

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

