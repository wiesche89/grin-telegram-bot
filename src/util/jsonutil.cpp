#include "jsonutil.h"

/**
 * @brief JsonUtil::extractOkObject
 * @param rootObj
 * @return
 */
Result<QJsonObject> JsonUtil::extractOkObject(const QJsonObject &rootObj)
{
    // Parse error from JSON
    Error error = Error::fromJson(rootObj);
    if (error.isError()) {
        return error;
    }

    // Check for "result" object
    if (!rootObj.contains("result") || !rootObj["result"].isObject()) {
        return Error(ErrorType::Unknown, "no result element!");
    }
    QJsonObject resultObj = rootObj["result"].toObject();

    // Check for "Ok" object inside "result"
    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return Error(ErrorType::Unknown, "no Ok element!");
    }
    QJsonObject okObj = resultObj["Ok"].toObject();

    return okObj;
}

/**
 * @brief JsonUtil::extractOkValue
 * @param rootObj
 * @return
 */
Result<QJsonValue> JsonUtil::extractOkValue(const QJsonObject &rootObj)
{
    // Parse error from JSON
    Error error = Error::fromJson(rootObj);
    if (error.isError()) {
        return error;
    }

    // Check for "result" object
    if (!rootObj.contains("result") || !rootObj["result"].isObject()) {
        return Error(ErrorType::Unknown, "no result element!");
    }
    QJsonObject resultObj = rootObj["result"].toObject();

    // Check for "Ok" key
    if (!resultObj.contains("Ok")) {
        return Error(ErrorType::Unknown, "no Ok element!");
    }

    // Return the value (could be object, array, int, string, etc.)
    QJsonValue okValue = resultObj["Ok"];
    return okValue;
}
