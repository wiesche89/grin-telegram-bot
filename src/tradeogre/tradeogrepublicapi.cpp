#include "tradeogrepublicapi.h"

TradeOgrePublicApi::TradeOgrePublicApi(QObject *parent) : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &TradeOgrePublicApi::onReplyFinished);
}

void TradeOgrePublicApi::getMarkets() {
    m_nam.get(QNetworkRequest(QUrl("https://tradeogre.com/api/v1/markets")));
}

void TradeOgrePublicApi::getOrderBook(const QString &market) {
    m_nam.get(QNetworkRequest(QUrl("https://tradeogre.com/api/v1/orders/" + market)));
}

void TradeOgrePublicApi::getTicker(const QString &market) {
    m_nam.get(QNetworkRequest(QUrl("https://tradeogre.com/api/v1/ticker/" + market)));
}

void TradeOgrePublicApi::getTradeHistory(const QString &market) {
    m_nam.get(QNetworkRequest(QUrl("https://tradeogre.com/api/v1/history/" + market)));
}

void TradeOgrePublicApi::getChart(const QString &interval, const QString &market, qint64 timestamp) {
    QString url = QString("https://tradeogre.com/api/v1/chart/%1/%2/%3").arg(interval, market).arg(timestamp);
    m_nam.get(QNetworkRequest(QUrl(url)));
}

void TradeOgrePublicApi::onReplyFinished(QNetworkReply *reply) {
    const QUrl url = reply->url();
    const QByteArray data = reply->readAll();
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

    if (url.path().contains("/markets")) {
        emit marketsReceived(doc.array());
    } else if (url.path().contains("/orders/")) {
        emit orderBookReceived(doc.object());
    } else if (url.path().contains("/ticker/")) {
        emit tickerReceived(doc.object());
    } else if (url.path().contains("/history/")) {
        emit tradeHistoryReceived(doc.array());
    } else if (url.path().contains("/chart/")) {
        emit chartDataReceived(doc.array());
    }
    reply->deleteLater();
}
