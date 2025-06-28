#include "summaryinfo.h"

SummaryInfo SummaryInfo::fromJson(const QJsonObject& obj) {
    SummaryInfo info;
    info.amountAwaitingConfirmation = obj["amount_awaiting_confirmation"].toString().toLongLong();
    info.amountAwaitingFinalization = obj["amount_awaiting_finalization"].toString().toLongLong();
    info.amountCurrentlySpendable = obj["amount_currently_spendable"].toString().toLongLong();
    info.amountImmature = obj["amount_immature"].toString().toLongLong();
    info.amountLocked = obj["amount_locked"].toString().toLongLong();
    info.amountReverted = obj["amount_reverted"].toString().toLongLong();
    info.lastConfirmedHeight = obj["last_confirmed_height"].toString().toLongLong();
    info.minimumConfirmations = obj["minimum_confirmations"].toString().toInt();
    info.total = obj["total"].toString().toLongLong();
    return info;
}

QJsonObject SummaryInfo::toJson() const {
    QJsonObject obj;
    obj["amount_awaiting_confirmation"] = QString::number(amountAwaitingConfirmation);
    obj["amount_awaiting_finalization"] = QString::number(amountAwaitingFinalization);
    obj["amount_currently_spendable"] = QString::number(amountCurrentlySpendable);
    obj["amount_immature"] = QString::number(amountImmature);
    obj["amount_locked"] = QString::number(amountLocked);
    obj["amount_reverted"] = QString::number(amountReverted);
    obj["last_confirmed_height"] = QString::number(lastConfirmedHeight);
    obj["minimum_confirmations"] = QString::number(minimumConfirmations);
    obj["total"] = QString::number(total);
    return obj;
}
