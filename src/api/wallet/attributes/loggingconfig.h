#ifndef LOGGINGCONFIG_H
#define LOGGINGCONFIG_H

#include <QString>
#include <QJsonObject>

class LoggingConfig
{
public:
    LoggingConfig();

    bool logToStdout() const;
    void setLogToStdout(bool value);

    QString stdoutLogLevel() const;
    void setStdoutLogLevel(const QString &value);

    bool logToFile() const;
    void setLogToFile(bool value);

    QString fileLogLevel() const;
    void setFileLogLevel(const QString &value);

    QString logFilePath() const;
    void setLogFilePath(const QString &value);

    bool logFileAppend() const;
    void setLogFileAppend(bool value);

    int logMaxSize() const;
    void setLogMaxSize(int value);

    int logMaxFiles() const;
    void setLogMaxFiles(int value);

    bool tuiRunning() const;
    void setTuiRunning(bool value);

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    bool m_logToStdout;
    QString m_stdoutLogLevel;
    bool m_logToFile;
    QString m_fileLogLevel;
    QString m_logFilePath;
    bool m_logFileAppend;
    int m_logMaxSize;
    int m_logMaxFiles;
    bool m_tuiRunning;
};

#endif // LOGGINGCONFIG_H
