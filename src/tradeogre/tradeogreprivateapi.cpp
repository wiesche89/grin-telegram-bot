#include "tradeogreprivateapi.h"

TradeOgrePrivateApi::TradeOgrePrivateApi(const QString &pub, const QString &priv, QObject *parent)
    : QObject(parent), m_publicKey(pub), m_privateKey(priv)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &TradeOgrePrivateApi::onReplyFinished);
}

QNetworkRequest TradeOgrePrivateApi::createRequest(const QString &endpoint) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/" + endpoint));
    // Basic Auth
    QString credentials = m_publicKey + ":" + m_privateKey;
    QByteArray auth = "Basic " + credentials.toUtf8().toBase64();
    req.setRawHeader("Authorization", auth);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return req;
}

void TradeOgrePrivateApi::post(const QString &endpoint, const QJsonObject &payload) {
    QUrlQuery query;
    for (auto it = payload.begin(); it != payload.end(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }
    QByteArray data = query.query(QUrl::FullyEncoded).toUtf8();
    m_nam.post(createRequest(endpoint), data);
}

// ===== API Calls =====
void TradeOgrePrivateApi::submitBuyOrder(const QString &market, const QString &quantity, const QString &price, const QString &duration) {
    post("order/buy", {{"market",market},{"quantity",quantity},{"price",price},{"duration",duration}});
}

void TradeOgrePrivateApi::submitSellOrder(const QString &market, const QString &quantity, const QString &price, const QString &duration) {
    post("order/sell", {{"market",market},{"quantity",quantity},{"price",price},{"duration",duration}});
}

void TradeOgrePrivateApi::cancelOrder(const QString &uuid) {
    post("order/cancel", {{"uuid",uuid}});
}

void TradeOgrePrivateApi::getOrders(const QString &market) {
    QJsonObject o; if(!market.isEmpty()) o.insert("market",market);
    post("account/orders", o);
}

void TradeOgrePrivateApi::getBalance(const QString &currency) {
    post("account/balance", {{"currency",currency}});
}

void TradeOgrePrivateApi::getBalances() {
    // GET request (no payload)
    m_nam.get(createRequest("account/balances"));
}

void TradeOgrePrivateApi::withdraw(const QString &currency, const QString &network, const QString &address, const QString &paymentid, const QString &quantity) {
    QJsonObject p{{"currency",currency},{"network",network},{"address",address},{"paymentid",paymentid},{"quantity",quantity}};
    post("account/withdraw", p);
}

void TradeOgrePrivateApi::onReplyFinished(QNetworkReply *reply) {
    const QString endpoint = reply->url().path();
    const QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        emit requestError(endpoint, reply->errorString());
        reply->deleteLater();
        return;
    }
    QJsonParseError jerr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jerr);
    if (jerr.error != QJsonParseError::NoError) {
        emit requestError(endpoint, jerr.errorString());
        reply->deleteLater();
        return;
    }
    emit privateReply(endpoint, doc);
    reply->deleteLater();
}
