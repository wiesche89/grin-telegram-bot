#ifndef OUTPUT_H
#define OUTPUT_H

#include <QString>
#include <QJsonObject>

class Output
{
public:
    Output();
    Output(const QString &commit, const QString &features, const QString &proof);

    QJsonObject toJson() const;
    static Output fromJson(const QJsonObject &obj);

    QString commit() const;
    void setCommit(const QString &commit);

    QString features() const;
    void setFeatures(const QString &features);

    QString proof() const;
    void setProof(const QString &proof);

private:
    QString m_commit;
    QString m_features;
    QString m_proof;
};

#endif // OUTPUT_H
