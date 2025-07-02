#ifndef JSONUTIL_H
#define JSONUTIL_H

#include <QJsonObject>
#include <QJsonArray>

#include "result.h"
#include "error.h"

class JsonUtil
{
public:
    // Static method to extract the "Ok" object or return an Error wrapped in Result
    static Result<QJsonObject> extractOkObject(const QJsonObject &rootObj);
    static Result<QJsonValue> extractOkValue(const QJsonObject &rootObj);
};

#endif // JSONUTIL_H
