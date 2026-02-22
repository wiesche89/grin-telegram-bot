#include "alivehandler.h"

#include "telegrambot/telegrambot.h"

#include <QDateTime>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <utility>

namespace {
constexpr int AliveIntervalMs = 10 * 60 * 1000; // 10 minutes
QStringList splitAdminIds(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty()) {
        return {};
    }

    QString cleaned = normalized;
    cleaned.remove('\"');
    return cleaned.split(',', Qt::SkipEmptyParts);
}
}

AliveHandler::AliveHandler(TelegramBot *bot, QSettings *settings, QObject *parent)
    : QObject(parent),
      m_bot(bot),
      m_settings(settings),
      m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &AliveHandler::sendAliveMessage);
    loadAdminIds();
}

void AliveHandler::start()
{
    if (m_adminIds.isEmpty() || !m_bot) {
        return;
    }

    sendAliveMessage();
    m_timer->start(AliveIntervalMs);
}

void AliveHandler::sendAliveMessage()
{
    if (m_adminIds.isEmpty() || !m_bot) {
        return;
    }

    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    const QString text = QStringLiteral("Alive check: %1").arg(timestamp);

    for (qlonglong adminId : std::as_const(m_adminIds)) {
        m_bot->sendMessage(adminId, text, 0, TelegramBot::NoFlag);
    }
}

void AliveHandler::loadAdminIds()
{
    if (!m_settings) {
        return;
    }

    const QStringList parts = splitAdminIds(m_settings->value("admin/telegramIds").toString());
    m_adminIds.clear();

    for (const QString &part : parts) {
        const QString trimmed = part.trimmed();
        bool ok = false;
        const qlonglong id = trimmed.toLongLong(&ok);
        if (ok) {
            m_adminIds.append(id);
        }
    }
}
