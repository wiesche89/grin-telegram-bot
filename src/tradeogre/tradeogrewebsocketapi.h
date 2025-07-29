#ifndef TRADEOGREWEBSOCKETAPI_H
#define TRADEOGREWEBSOCKETAPI_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

/**
 * @brief TradeOgre WebSocket API Client
 *        verbindet sich mit wss://tradeogre.com:8443
 *        und liefert Echtzeit-Orderbook- und Trade-Updates
 */
class TradeOgreWebSocketApi : public QObject
{
    Q_OBJECT
public:
    explicit TradeOgreWebSocketApi(QObject *parent = nullptr);

    /// Verbindung zum TradeOgre WebSocket herstellen
    void connectSocket();

    /// Verbindung trennen
    void disconnectSocket();

    /// Orderbook-Stream für einen Markt abonnieren (z.B. "XMR-BTC")
    void subscribeOrderBook(const QString &market);

    /// Trade-Stream abonnieren ("*" für alle Märkte)
    void subscribeTrades(const QString &market = "*");

signals:
    /// Signal für empfangenes Orderbook-Update
    void orderBookUpdate(const QJsonObject &update);

    /// Signal für empfangene Trades
    void tradeUpdate(const QJsonObject &update);

    /// Signal für Fehler
    void errorOccurred(const QString &message);

    /// Signal bei erfolgreicher Verbindung
    void connected();

    /// Signal wenn Verbindung getrennt wurde
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket m_socket;
    bool m_isConnected = false;
};

#endif // TRADEOGREWEBSOCKETAPI_H
