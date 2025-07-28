#ifndef TRADEOGREPUBLICAPI_H
#define TRADEOGREPUBLICAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class TradeOgrePublicApi : public QObject
{
    Q_OBJECT
public:
    explicit TradeOgrePublicApi(QObject *parent = nullptr);

    void getMarkets();
    void getOrderBook(const QString &market);
    void getTicker(const QString &market);
    void getTradeHistory(const QString &market);
    void getChart(const QString &interval, const QString &market, qint64 timestamp);

signals:
    void marketsReceived(const QJsonArray &markets);
    void orderBookReceived(const QJsonObject &book);
    void tickerReceived(const QJsonObject &ticker);
    void tradeHistoryReceived(const QJsonArray &history);
    void chartDataReceived(const QJsonArray &chart);

    void requestError(const QString &endpoint, const QString &errorString);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
};


#endif // TRADEOGREPUBLICAPI_H
