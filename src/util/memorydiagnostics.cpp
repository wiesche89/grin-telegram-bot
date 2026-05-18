#include "memorydiagnostics.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>

#include <atomic>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

Q_LOGGING_CATEGORY(memoryDiag, "bot.memory")

namespace {
std::atomic<int> g_activeNetworkReplies {0};
}

MemoryDiagnostics::MemoryDiagnostics(QObject *parent) :
    QObject(parent)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(10 * 60 * 1000);
    connect(timer, &QTimer::timeout, this, &MemoryDiagnostics::logSnapshot);
    timer->start();

    QTimer::singleShot(0, this, &MemoryDiagnostics::logSnapshot);
}

void MemoryDiagnostics::trackNetworkReply(QNetworkReply *reply, const QString &tag)
{
    if (!reply) {
        return;
    }

    g_activeNetworkReplies.fetch_add(1, std::memory_order_relaxed);
    QObject::connect(reply, &QNetworkReply::finished, reply, [tag]() {
        const int active = g_activeNetworkReplies.fetch_sub(1, std::memory_order_relaxed) - 1;
        if (!tag.isEmpty()) {
            qCDebug(memoryDiag) << "network reply finished" << tag << "activeReplies" << active;
        }
    });
}

int MemoryDiagnostics::activeNetworkReplies()
{
    return g_activeNetworkReplies.load(std::memory_order_relaxed);
}

qint64 MemoryDiagnostics::currentRssKb()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX counters;
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&counters),
                             sizeof(counters))) {
        return static_cast<qint64>(counters.WorkingSetSize / 1024);
    }
    return -1;
#else
    QFile statusFile("/proc/self/status");
    if (!statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QTextStream in(&statusFile);
    const QRegularExpression vmRssPattern("^VmRSS:\\s+(\\d+)\\s+kB");
    while (!in.atEnd()) {
        const QString line = in.readLine();
        const QRegularExpressionMatch match = vmRssPattern.match(line);
        if (match.hasMatch()) {
            return match.captured(1).toLongLong();
        }
    }
    return -1;
#endif
}

void MemoryDiagnostics::logSnapshot()
{
    qCInfo(memoryDiag) << "snapshot"
                       << "rssKb" << currentRssKb()
                       << "activeNetworkReplies" << activeNetworkReplies()
                       << "applicationPid" << QCoreApplication::applicationPid();
}
