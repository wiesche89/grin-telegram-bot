#ifndef MEMORYDIAGNOSTICS_H
#define MEMORYDIAGNOSTICS_H

#include <QObject>
#include <QString>

class QNetworkReply;

class MemoryDiagnostics : public QObject
{
    Q_OBJECT

public:
    explicit MemoryDiagnostics(QObject *parent = nullptr);

    static void trackNetworkReply(QNetworkReply *reply, const QString &tag = QString());
    static int activeNetworkReplies();
    static qint64 currentRssKb();

public slots:
    void logSnapshot();
};

#endif // MEMORYDIAGNOSTICS_H
