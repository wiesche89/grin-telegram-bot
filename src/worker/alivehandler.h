#ifndef WORKER_ALIVEHANDLER_H
#define WORKER_ALIVEHANDLER_H

#include <QList>
#include <QObject>

class QSettings;
class QTimer;
class TelegramBot;

class AliveHandler : public QObject
{
    Q_OBJECT

public:
    AliveHandler(TelegramBot *bot, QSettings *settings, QObject *parent = nullptr);
    ~AliveHandler() override = default;

    void start();

private slots:
    void sendAliveMessage();

private:
    void loadAdminIds();

    TelegramBot *m_bot;
    QSettings *m_settings;
    QList<qlonglong> m_adminIds;
    QTimer *m_timer;
};

#endif // WORKER_ALIVEHANDLER_H
