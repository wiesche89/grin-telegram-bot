#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include <QJsonObject>
#include <QJsonValue>

enum class ErrorType {
    RpcError,       // JSON-RPC Standard
    ApiInternal,    // Intern (API)
    ApiOther,
    NoError,
    Unknown
};

class Error
{
public:
    ErrorType type;
    QString message;
    int code; // only RpcError

    Error(ErrorType t = ErrorType::NoError, const QString &msg = QString(), int c = 0);
    bool isError() const;

    static Error fromJson(const QJsonObject &obj);
};

#endif // ERROR_H
