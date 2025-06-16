#include "slate.h"

QJsonObject Slate::toJson() const {
    QJsonObject obj;
    obj["amt"] = amt;
    obj["fee"] = fee;
    obj["id"] = id;

    QJsonArray sigArray;
    for (const auto& sig : sigs) {
        sigArray.append(sig.toJson());
    }
    obj["sigs"] = sigArray;

    obj["sta"] = sta;
    obj["ver"] = ver;

    return obj;
}

Slate Slate::fromJson(const QJsonObject& obj) {
    Slate slate;
    slate.amt = obj["amt"].toString();
    slate.fee = obj["fee"].toString();
    slate.id = obj["id"].toString();

    QJsonArray sigArray = obj["sigs"].toArray();
    for (const auto& sigVal : sigArray) {
        slate.sigs.append(Signature::fromJson(sigVal.toObject()));
    }

    slate.sta = obj["sta"].toString();
    slate.ver = obj["ver"].toString();

    return slate;
}
