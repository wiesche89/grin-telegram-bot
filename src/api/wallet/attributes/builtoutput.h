#ifndef BUILTOUTPUT_H
#define BUILTOUTPUT_H

#include <QString>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

class BuiltOutput
{
public:
    BuiltOutput();

    QString blind() const;
    void setBlind(const QString &blind);

    QString keyId() const;
    void setKeyId(const QString &keyId);

    QString commit() const;
    void setCommit(const QString &commit);

    QString features() const;
    void setFeatures(const QString &features);

    QString proof() const;
    void setProof(const QString &proof);

    static BuiltOutput fromJson(const QJsonObject &rootObj);
    QJsonObject toJson() const;

private:
    QString m_blind;
    QString m_keyId;
    QString m_commit;
    QString m_features;
    QString m_proof;
};

#endif // BUILTOUTPUT_H
