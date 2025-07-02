#include "error.h"

Error::Error(ErrorType t, const QString &msg, int c) :
    type(t),
    message(msg),
    code(c)
{
}

bool Error::isError() const
{
    return type != ErrorType::NoError;
}

Error Error::fromJson(const QJsonObject &obj)
{
    /** JSON-RPC
    {
        "error": {
            "code": -32602,
            "message": "WrongNumberOfArgs. Expected 4. Actual 5"
        },
                  "id": 1,
                  "jsonrpc": "2.0"
    }
    **/
    if (obj.contains("error") && obj["error"].isObject()) {
        QJsonObject errObj = obj["error"].toObject();
        int code = errObj["code"].toInt();
        QString message = errObj["message"].toString();
        return Error(ErrorType::RpcError, message, code);
    }

    /** API Result
    {
        "id": 2,
            "jsonrpc": "2.0",
                        "result": {
            "Err": {
                "Internal": "ban peer error: PeerNotFound"
            }
        }
    }
    **/
    if (obj.contains("result") && obj["result"].isObject()) {
        QJsonObject resultObj = obj["result"].toObject();
        if (resultObj.contains("Err")) {
            QJsonValue errVal = resultObj["Err"];
            if (errVal.isObject()) {
                QJsonObject errDetail = errVal.toObject();
                if (errDetail.contains("Internal")) {
                    return Error(ErrorType::ApiInternal, errDetail["Internal"].toString());
                }
            } else if (errVal.isString()) {
                return Error(ErrorType::ApiOther, errVal.toString());
            }
        }
    }

    // No Error
    return Error(ErrorType::NoError, QString());
}
