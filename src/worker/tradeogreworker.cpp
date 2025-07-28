#include "tradeogreworker.h"

/**
 * @brief TradeOgreWorker::TradeOgreWorker
 * @param bot
 * @param settings
 */
TradeOgreWorker::TradeOgreWorker(TelegramBot *bot, QSettings *settings) :
    m_bot(bot),
    m_settings(settings),
    m_publicApi(nullptr),
    m_privateApi(nullptr),
    m_wsApi(nullptr)
{
}

/**
 * @brief TradeOgreWorker::init
 * @param pubKey
 * @param privKey
 * @return
 */
bool TradeOgreWorker::init(const QString &pubKey, const QString &privKey)
{
    // Public API
    if (!m_publicApi) {
        m_publicApi = new TradeOgrePublicApi(this);
        connect(m_publicApi, &TradeOgrePublicApi::requestError, this, [](const QString &ep, const QString &err) {
            qWarning() << "[PublicAPI]" << ep << err;
        });
    }

    // Private API
    if (!m_privateApi) {
        m_privateApi = new TradeOgrePrivateApi(pubKey, privKey, this);
        connect(m_privateApi, &TradeOgrePrivateApi::requestError, this, [](const QString &ep, const QString &err) {
            qWarning() << "[PrivateAPI]" << ep << err;
        });
    }

    // WebSocket API
    if (!m_wsApi) {
        m_wsApi = new TradeOgreWebSocketApi(this);
        connect(m_wsApi, &TradeOgreWebSocketApi::errorOccurred, this, [](const QString &err) {
            qWarning() << "[WebSocketAPI]" << err;
        });
        connect(m_wsApi, &TradeOgreWebSocketApi::connected, this, []() {
            qDebug() << "[WebSocketAPI] Connected";
        });
        connect(m_wsApi, &TradeOgreWebSocketApi::orderBookUpdate, this, [](const QJsonObject &obj) {
            qDebug() << "[WS OrderBook]" << obj;
        });
        connect(m_wsApi, &TradeOgreWebSocketApi::tradeUpdate, this, [](const QJsonObject &obj) {
            qDebug() << "[WS Trade]" << obj;
        });

        m_wsApi->connectSocket();
    }

    qDebug() << "TradeOgreManager initialized";

    // Set Slot to bot message
    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));

    return true;
}

/**
 * @brief TradeOgreWorker::onMessage
 * @param update
 */
void TradeOgreWorker::onMessage(TelegramBotUpdate update)
{
    if (update->type != TelegramBotMessageType::Message) {
        return;
    }

    TelegramBotMessage &message = *update->message;

    // -------- /ticker --------
    if (message.text.startsWith("/priceusdt")) {
        m_publicApi->getTicker("GRIN-USDT");

        connect(m_publicApi, &TradeOgrePublicApi::tickerReceived, this,
                [this, message](const QJsonObject &ticker) {
            QString txt;
            if (ticker.contains("price")) {
                txt = QString("Market: %1\nPrice: %2\nHigh: %3\nLow: %4\nVolume: %5\nBid: %6\nAsk: %7")
                      .arg("GRIN-USDT",
                           ticker.value("price").toString(),
                           ticker.value("high").toString(),
                           ticker.value("low").toString(),
                           ticker.value("volume").toString(),
                           ticker.value("bid").toString(),
                           ticker.value("ask").toString());
            }
            sendUserMessage(message, txt, false);
        }, Qt::SingleShotConnection);
        return;
    }
    if (message.text.startsWith("/pricebtc")) {
        m_publicApi->getTicker("GRIN-BTC");

        connect(m_publicApi, &TradeOgrePublicApi::tickerReceived, this,
                [this, message](const QJsonObject &ticker) {
            QString txt;
            if (ticker.contains("price")) {
                txt = QString("Market: %1\nPrice: %2\nHigh: %3\nLow: %4\nVolume: %5\nBid: %6\nAsk: %7")
                      .arg("GRIN-BTC",
                           ticker.value("price").toString(),
                           ticker.value("high").toString(),
                           ticker.value("low").toString(),
                           ticker.value("volume").toString(),
                           ticker.value("bid").toString(),
                           ticker.value("ask").toString());
            }
            sendUserMessage(message, txt, false);
        }, Qt::SingleShotConnection);
        return;
    }

    // -------- /orderbook --------
    if (message.text.startsWith("/orderbook")) {
        auto handleBook = [this, message](const QJsonObject &book) {
                              if (!book.value("success").toBool()) {
                                  sendUserMessage(message, "Error retrieving orderbook", false);
                                  return;
                              }
                              QString txt;
                              txt += "=== BUY (Top 10) ===\n";
                              int count = 0;
                              for (auto it = book.value("buy").toObject().begin(); it != book.value("buy").toObject().end() && count < 10;
                                   ++it, ++count) {
                                  txt += QString("%1 : %2\n").arg(it.key(), it.value().toString());
                              }
                              txt += "\n=== SELL (Top 10) ===\n";
                              count = 0;
                              for (auto it = book.value("sell").toObject().begin(); it != book.value("sell").toObject().end() && count < 10;
                                   ++it, ++count) {
                                  txt += QString("%1 : %2\n").arg(it.key(), it.value().toString());
                              }
                              sendUserMessage(message, txt, false);
                          };

        connect(m_publicApi, &TradeOgrePublicApi::orderBookReceived, this, handleBook);
        m_publicApi->getOrderBook("GRIN-USDT");
        m_publicApi->getOrderBook("GRIN-BTC");
        return;
    }

    // -------- /chart --------
    if (message.text.startsWith("/chart")) {
        qint64 now = QDateTime::currentSecsSinceEpoch();

        auto sendChart = [this, message](const QString &market, const QJsonArray &chart) {
                             QString path = renderChartToFile(chart, market);
                             if (!path.isEmpty()) {
                                 m_bot->sendPhoto(
                                     message.chat.id,
                                     path,
                                     QString("%1 4h Chart").arg(market),
                                     0,
                                     TelegramBot::NoFlag,
                                     TelegramKeyboardRequest(),
                                     nullptr
                                     );
                             } else {
                                 sendUserMessage(message, QString("Konnte Chart für %1 nicht generieren.").arg(market), false);
                             }
                         };

        connect(m_publicApi, &TradeOgrePublicApi::chartDataReceived, this,
                [sendChart](const QJsonArray &chart) {
            sendChart("GRIN-USDT", chart);
        }, Qt::SingleShotConnection);

        m_publicApi->getChart("4h", "GRIN-USDT", now);
        return;
    }

    // -------- /history --------
    if (message.text.startsWith("/history")) {
        auto handleHistory = [this, message](const QJsonArray &history) {
                                 QString txt;
                                 txt += "=== Recent Trades ===\n";
                                 for (auto t : history) {
                                     QJsonObject obj = t.toObject();
                                     txt += QString("%1 %2 %3 @ %4\n")
                                            .arg(QString::number(obj.value("date").toInt()),
                                                 obj.value("type").toString(),
                                                 obj.value("quantity").toString(),
                                                 obj.value("price").toString());
                                 }
                                 sendUserMessage(message, txt, false);
                             };

        connect(m_publicApi, &TradeOgrePublicApi::tradeHistoryReceived, this, handleHistory);
        m_publicApi->getTradeHistory("GRIN-USDT");
        return;
    }
}

/**
 * @brief TradeOgreWorker::sendUserMessage
 * @param message
 * @param content
 * @param plain
 */
void TradeOgreWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi "
                      + message.from.firstName
                      + ",\n"
                      + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}

/**
 * @brief TradeOgreWorker::renderChartToFile
 * @param chart
 * @param market
 * @return
 */
QString TradeOgreWorker::renderChartToFile(const QJsonArray &chart, const QString &market)
{
    const int width = 900;
    const int height = 500;
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(QColor("#222222")); // dark background

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);

    int axisLeft = 60;
    int axisBottom = height - 40;
    int axisTop = 20;
    int axisRight = width - 20;

    // Frame
    p.setPen(QPen(Qt::white, 1));
    p.drawRect(0, 0, width - 1, height - 1);

    const int candleCount = qMin(chart.size(), 300);
    struct Candle {
        qint64 ts;
        double open, high, low, close;
    };
    QVector<Candle> candles;
    candles.reserve(candleCount);

    double minPrice = std::numeric_limits<double>::max();
    double maxPrice = std::numeric_limits<double>::lowest();

    for (int i = chart.size() - candleCount; i < chart.size(); ++i) {
        QJsonArray c = chart.at(i).toArray();
        if (c.size() >= 6) {
            qint64 ts = c.at(0).toVariant().toLongLong();
            double open = c.at(1).toDouble();
            double high = c.at(2).toDouble();
            double low = c.at(3).toDouble();
            double close = c.at(4).toDouble();
            candles.push_back({ts, open, high, low, close});
            minPrice = qMin(minPrice, low);
            maxPrice = qMax(maxPrice, high);
        }
    }
    if (candles.isEmpty()) {
        return QString();
    }

    double priceRange = maxPrice - minPrice;
    if (priceRange <= 0.0) {
        priceRange = 1.0;
    }
    double candleWidth = (double)(axisRight - axisLeft) / candles.size();

    // Axes
    p.setPen(QPen(Qt::white, 1, Qt::DashLine));
    p.drawLine(axisLeft, axisBottom, axisRight, axisBottom);
    p.drawLine(axisLeft, axisBottom, axisLeft, axisTop);

    // Y-Axes
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 10));
    int ySteps = 8;
    for (int i = 0; i <= ySteps; ++i) {
        double price = minPrice + (priceRange / ySteps) * i;
        int y = (int)(axisBottom - ((price - minPrice) / priceRange) * (axisBottom - axisTop));
        p.drawLine(axisLeft - 5, y, axisLeft, y);
        p.drawText(2, y + 4, QString::number(price, 'f', 2));
    }

    // X-Axes
    int xLabels = 8;
    for (int i = 0; i <= xLabels; ++i) {
        int idx = (candles.size() - 1) * i / xLabels;
        double x = axisLeft + idx * candleWidth;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(candles[idx].ts);
        QString label = dt.toString("dd.MM"); // day and month
        p.drawLine((int)x, axisBottom, (int)x, axisBottom + 5);
        p.drawText((int)x - 20, axisBottom + 18, label);
    }

    // Candles
    for (int i = 0; i < candles.size(); ++i) {
        double x = axisLeft + i * candleWidth;
        double o = candles[i].open;
        double h = candles[i].high;
        double l = candles[i].low;
        double c = candles[i].close;

        auto toY = [&](double price) {
                       return (int)(axisBottom - ((price - minPrice) / priceRange) * (axisBottom - axisTop));
                   };

        int yHigh = toY(h);
        int yLow = toY(l);
        int yOpen = toY(o);
        int yClose = toY(c);

        QColor bodyColor = (c >= o) ? QColor("#00FF00") : QColor("#FF4040");
        p.setPen(QPen(Qt::white, 1));
        p.setBrush(bodyColor);

        p.drawLine(QPointF(x + candleWidth / 2, yHigh),
                   QPointF(x + candleWidth / 2, yLow));

        int top = qMin(yOpen, yClose);
        int bottom = qMax(yOpen, yClose);
        QRectF bodyRect(x - candleWidth * 0.4, top, candleWidth * 0.8, bottom - top);
        p.drawRect(bodyRect);
    }

    // Titel
    p.setPen(Qt::yellow);
    p.setFont(QFont("Arial", 14, QFont::Bold));
    p.drawText(QRect(0, 0, width, 20), Qt::AlignCenter, market + QStringLiteral(" 4h Chart"));

    p.end();

    // Save
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(tempPath);
    QString filePath = tempPath + "/" + market + "_chart.png";
    if (image.save(filePath, "PNG")) {
        return filePath;
    }
    return QString();
}
