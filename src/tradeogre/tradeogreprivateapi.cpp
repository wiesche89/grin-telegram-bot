#include "tradeogreprivateapi.h"

/**
 * @brief Constructor for TradeOgrePrivateApi
 * @param pub Public API key
 * @param priv Private API key
 * @param parent Optional QObject parent
 */
TradeOgrePrivateApi::TradeOgrePrivateApi(const QString &pub, const QString &priv, QObject *parent)
    : QObject(parent), m_publicKey(pub), m_privateKey(priv)
{
    // Connect the network reply handler
    connect(&m_nam, &QNetworkAccessManager::finished, this, &TradeOgrePrivateApi::onReplyFinished);
}

/**
 * @brief Creates a QNetworkRequest with Basic Auth and proper headers
 * @param endpoint API endpoint (e.g. "account/balances")
 * @return Configured QNetworkRequest
 */
QNetworkRequest TradeOgrePrivateApi::createRequest(const QString &endpoint) {
    QNetworkRequest req(QUrl("https://tradeogre.com/api/v1/" + endpoint));

    // Basic Auth Header (public:private base64 encoded)
    QString credentials = m_publicKey + ":" + m_privateKey;
    QByteArray auth = "Basic " + credentials.toUtf8().toBase64();
    req.setRawHeader("Authorization", auth);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return req;
}

/**
 * @brief Performs a POST request with URL-encoded form data
 * @param endpoint API endpoint
 * @param payload JSON key-value pairs to send
 */
void TradeOgrePrivateApi::post(const QString &endpoint, const QJsonObject &payload) {
    QUrlQuery query;
    for (auto it = payload.begin(); it != payload.end(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }
    QByteArray data = query.query(QUrl::FullyEncoded).toUtf8();
    m_nam.post(createRequest(endpoint), data);
}

// ===== TradeOgre Private API Methods =====

/**
 * @brief Submits a buy order
 */
void TradeOgrePrivateApi::submitBuyOrder(const QString &market, const QString &quantity, const QString &price, const QString &duration) {
    post("order/buy", {{"market",market},{"quantity",quantity},{"price",price},{"duration",duration}});
}

/**
 * @brief Submits a sell order
 */
void TradeOgrePrivateApi::submitSellOrder(const QString &market, const QString &quantity, const QString &price, const QString &duration) {
    post("order/sell", {{"market",market},{"quantity",quantity},{"price",price},{"duration",duration}});
}

/**
 * @brief Cancels an open order by UUID
 */
void TradeOgrePrivateApi::cancelOrder(const QString &uuid) {
    post("order/cancel", {{"uuid",uuid}});
}

/**
 * @brief Fetches current open orders (optionally filtered by market)
 */
void TradeOgrePrivateApi::getOrders(const QString &market) {
    QJsonObject o;
    if (!market.isEmpty()) o.insert("market", market);
    post("account/orders", o);
}

/**
 * @brief Fetches balance for a single currency
 */
void TradeOgrePrivateApi::getBalance(const QString &currency) {
    post("account/balance", {{"currency",currency}});
}

/**
 * @brief Fetches all balances (no payload, uses GET)
 */
void TradeOgrePrivateApi::getBalances() {
    m_nam.get(createRequest("account/balances"));
}

/**
 * @brief Initiates a withdrawal from the account
 */
void TradeOgrePrivateApi::withdraw(const QString &currency, const QString &network, const QString &address, const QString &paymentid, const QString &quantity) {
    QJsonObject p{
        {"currency", currency},
        {"network", network},
        {"address", address},
        {"paymentid", paymentid},
        {"quantity", quantity}
    };
    post("account/withdraw", p);
}

/**
 * @brief Handles the finished network reply
 * Parses JSON and emits privateReply or requestError
 */
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
