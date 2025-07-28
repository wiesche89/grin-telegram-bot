#include "tradeogreworker.h"

/**
 * @brief TradeOgreWorker::TradeOgreWorker
 * @param bot
 * @param settings
 */
TradeOgreWorker::TradeOgreWorker(TelegramBot *bot, QSettings *settings) :
    m_bot(bot),
    m_settings(settings),
    m_publicApi(nullptr)
{
}

bool TradeOgreWorker::init()
{
    // Public API
    if (!m_publicApi) {
        m_publicApi = new TradeOgrePublicApi(this);
        connect(m_publicApi, &TradeOgrePublicApi::requestError, this, [](const QString &ep, const QString &err) {
            qWarning() << "[PublicAPI]" << ep << err;
        });
    }

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
    if (update->type != TelegramBotMessageType::Message) return;
    TelegramBotMessage &message = *update->message;

    //--------- /price-------------
    if (message.text.startsWith("/price")) {
        struct TickerStore {
            bool usdtDone = false;
            bool btcDone = false;
            QJsonObject usdt;
            QJsonObject btc;
        };

        TickerStore* store = new TickerStore();
        QMetaObject::Connection* conn = new QMetaObject::Connection;

        *conn = connect(m_publicApi, &TradeOgrePublicApi::tickerReceived, this,
                        [this, message, store, conn](const QString &market, const QJsonObject &ticker) mutable {
                            if (market == "GRIN-USDT") {
                                store->usdt = ticker;
                                store->usdtDone = true;
                            } else if (market == "GRIN-BTC") {
                                store->btc = ticker;
                                store->btcDone = true;
                            }

                            if (store->usdtDone && store->btcDone) {
                                disconnect(*conn);
                                delete conn;

                                auto pct = [](double current, double ref) -> QString {
                                    if (ref == 0.0) return "n/a";
                                    double diff = ((current - ref) / ref) * 100.0;
                                    return QString("%1%2%").arg(diff >= 0 ? "+" : "").arg(QString::number(diff, 'f', 2));
                                };

                                auto toText = [&](const QString &market, const QJsonObject &ticker) -> QStringList {
                                    double price = ticker.value("price").toString().toDouble();
                                    double initialPrice = ticker.value("initialprice").toString().toDouble();
                                    double high = ticker.value("high").toString().toDouble();
                                    double low = ticker.value("low").toString().toDouble();
                                    double volume = ticker.value("volume").toString().toDouble();
                                    double bid = ticker.value("bid").toString().toDouble();
                                    double ask = ticker.value("ask").toString().toDouble();

                                    return {
                                        QString("[%1]").arg(market),
                                        QString("Price:   %1   (%2)").arg(QString::number(price, 'f', 8), -12).arg(pct(price, initialPrice)),
                                        QString("High:    %1   (%2)").arg(QString::number(high, 'f', 8), -12).arg(pct(high, initialPrice)),
                                        QString("Low:     %1   (%2)").arg(QString::number(low, 'f', 8), -12).arg(pct(low, initialPrice)),
                                        QString("Volume:  %1").arg(QString::number(volume, 'f', 4)),
                                        QString("Bid:     %1").arg(QString::number(bid, 'f', 8)),
                                        QString("Ask:     %1").arg(QString::number(ask, 'f', 8)),
                                        ""
                                    };
                                };

                                QStringList lines;
                                lines << toText("GRIN-USDT", store->usdt);
                                lines << toText("GRIN-BTC", store->btc);

                                sendUserMessage(message, lines.join("\n"), false);
                                delete store;
                            }
                        });

        m_publicApi->getTicker("GRIN-USDT");
        m_publicApi->getTicker("GRIN-BTC");
        return;
    }

    // -------- /orderbook --------
    if (message.text.startsWith("/orderbook")) {
        struct OrderBookStore {
            bool usdtDone = false;
            bool btcDone = false;
            QJsonObject usdt;
            QJsonObject btc;
        };

        OrderBookStore* store = new OrderBookStore();
        QMetaObject::Connection* conn = new QMetaObject::Connection;

        *conn = connect(m_publicApi, &TradeOgrePublicApi::orderBookReceived, this,
                        [this, message, store, conn](const QString &market, const QJsonObject &book) mutable {
                            if (!book.value("success").toBool())
                                return;

                            if (market == "GRIN-USDT") {
                                store->usdt = book;
                                store->usdtDone = true;
                            } else if (market == "GRIN-BTC") {
                                store->btc = book;
                                store->btcDone = true;
                            }

                            if (!store->usdtDone || !store->btcDone)
                                return;

                            disconnect(*conn);
                            delete conn;

                            auto renderOrderBook = [](const QString& market, const QJsonObject& book) -> QString {
                                std::map<double, QString, std::greater<double>> sortedBuy;
                                QJsonObject buyObj = book.value("buy").toObject();
                                for (auto it = buyObj.begin(); it != buyObj.end(); ++it)
                                    sortedBuy[it.key().toDouble()] = it.value().toString();

                                std::map<double, QString> sortedSell;
                                QJsonObject sellObj = book.value("sell").toObject();
                                for (auto it = sellObj.begin(); it != sellObj.end(); ++it)
                                    sortedSell[it.key().toDouble()] = it.value().toString();

                                QString txt;
                                txt += "=========== ORDERBOOK ===========\n\n";
                                txt += "[" + market + "]\n\n";

                                // BUY Orders (Top 10 - Highest Bids)
                                txt += "BUY Orders (Top 10 - Highest Bids):\n";
                                txt += "  Price        ×     Quantity\n";
                                txt += "-------------------------------\n";
                                QVector<QPair<double, QString>> topBuy;
                                int count = 0;
                                for (auto it = sortedBuy.begin(); it != sortedBuy.end() && count < 10; ++it, ++count)
                                    topBuy.append(qMakePair(it->first, it->second));
                                for (int i = topBuy.size() - 1; i >= 0; --i) {
                                    txt += QString("  %1     × %2\n")
                                               .arg(QString::number(topBuy[i].first, 'f', 8), -12)
                                               .arg(topBuy[i].second);
                                }

                                // SELL Orders (Top 10 - Lowest Asks)
                                txt += "\nSELL Orders (Top 10 - Lowest Asks):\n";
                                txt += "  Price        ×     Quantity\n";
                                txt += "-------------------------------\n";
                                count = 0;
                                for (auto it = sortedSell.begin(); it != sortedSell.end() && count < 10; ++it, ++count) {
                                    txt += QString("  %1     × %2\n")
                                               .arg(QString::number(it->first, 'f', 8), -12)
                                               .arg(it->second);
                                }

                                txt += "\n=================================\n";
                                return txt;
                            };

                            QString fullText;
                            fullText += renderOrderBook("GRIN-USDT", store->usdt);
                            fullText += "\n\n";
                            fullText += renderOrderBook("GRIN-BTC", store->btc);

                            sendUserMessage(message, fullText, false);
                            delete store;
                        });

        m_publicApi->getOrderBook("GRIN-USDT");
        m_publicApi->getOrderBook("GRIN-BTC");
        return;
    }

    // -------- /chart ------------
    if (message.text.startsWith("/chart")) {
        qint64 now = QDateTime::currentSecsSinceEpoch();

        struct ChartStore {
            bool usdtDone = false;
            bool btcDone = false;
            QJsonArray usdtChart;
            QJsonArray btcChart;
        };

        ChartStore* store = new ChartStore();
        QMetaObject::Connection* conn = new QMetaObject::Connection;

        *conn = connect(m_publicApi, &TradeOgrePublicApi::chartDataReceived, this,
                        [this, message, store, conn](const QString &market, const QJsonArray &chart) mutable {
                            if (market == "GRIN-USDT") {
                                store->usdtChart = chart;
                                store->usdtDone = true;
                            } else if (market == "GRIN-BTC") {
                                store->btcChart = chart;
                                store->btcDone = true;
                            }

                            if (!store->usdtDone || !store->btcDone)
                                return;

                            disconnect(*conn);
                            delete conn;

                            QString usdtPath = renderChartToFile(store->usdtChart, "GRIN-USDT");
                            QString btcPath  = renderChartToFile(store->btcChart, "GRIN-BTC");

                            if (!usdtPath.isEmpty())
                                m_bot->sendPhoto(message.chat.id, usdtPath, "GRIN-USDT 4h Chart");
                            else
                                sendUserMessage(message, "Konnte Chart für GRIN-USDT nicht generieren.", false);

                            if (!btcPath.isEmpty())
                                m_bot->sendPhoto(message.chat.id, btcPath, "GRIN-BTC 4h Chart");
                            else
                                sendUserMessage(message, "Konnte Chart für GRIN-BTC nicht generieren.", false);

                            delete store;
                        });

        m_publicApi->getChart("4h", "GRIN-USDT", now);
        m_publicApi->getChart("4h", "GRIN-BTC", now);
        return;
    }

    // -------- /history --------
    if (message.text.startsWith("/history")) {
        struct HistoryStore {
            bool usdtDone = false;
            bool btcDone = false;
            QJsonArray usdtHistory;
            QJsonArray btcHistory;
        };

        HistoryStore* store = new HistoryStore();
        QMetaObject::Connection* conn = new QMetaObject::Connection;

        *conn = connect(m_publicApi, &TradeOgrePublicApi::tradeHistoryReceived, this,
                        [this, message, store, conn](const QString &market, const QJsonArray &history) mutable {
                            if (market == "GRIN-USDT") {
                                store->usdtHistory = history;
                                store->usdtDone = true;
                            } else if (market == "GRIN-BTC") {
                                store->btcHistory = history;
                                store->btcDone = true;
                            }

                            if (!store->usdtDone || !store->btcDone)
                                return;

                            disconnect(*conn);
                            delete conn;

                            auto formatTrades = [](const QString& market, const QJsonArray& trades) -> QString {
                                QStringList lines;
                                lines << QString("=== Trade History for %1 ===").arg(market);
                                lines << QString("  Date                Type     Quantity         Price");

                                int shown = 0;
                                for (const QJsonValue &v : trades) {
                                    if (!v.isObject()) continue;
                                    const QJsonObject obj = v.toObject();

                                    QDateTime dt = QDateTime::fromSecsSinceEpoch(obj.value("date").toInt());
                                    QString timeStr = dt.toString("yyyy-MM-dd hh:mm:ss");

                                    QString type = obj.value("type").toString().toUpper();
                                    QString qty  = obj.value("quantity").toString();
                                    QString prc  = obj.value("price").toString();

                                    lines << QString("  %1   %2   %3   @ %4")
                                                 .arg(timeStr, -20)
                                                 .arg(type, -7)
                                                 .arg(qty, -15)
                                                 .arg(prc);

                                    if (++shown >= 12) break; // max 12 Zeilen pro Markt
                                }
                                return lines.join("\n");
                            };

                            QString result;
                            result += formatTrades("GRIN-USDT", store->usdtHistory);
                            result += "\n\n";
                            result += formatTrades("GRIN-BTC", store->btcHistory);

                            sendUserMessage(message, result, false);
                            delete store;
                        });

        m_publicApi->getTradeHistory("GRIN-USDT");
        m_publicApi->getTradeHistory("GRIN-BTC");
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

    int axisLeft = market.contains("BTC") ? 120 : 60;
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
        int precision = (market.contains("BTC")) ? 8 : 4;
        p.drawText(2, y + 4, QString::number(price, 'f', precision));
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
