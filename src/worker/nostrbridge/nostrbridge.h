#ifndef NOSTRBRIDGE_H
#define NOSTRBRIDGE_H

#include <QObject>
#include <QUrl>
#include <QSet>
#include <QStringList>
#include <QList>

#include <qnostr.h>

class NostrBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString secretKey READ secretKey NOTIFY keysChanged)
    Q_PROPERTY(QString publicKey READ publicKey NOTIFY keysChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString lastEventText READ lastEventText NOTIFY lastEventTextChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectionChanged)
    Q_PROPERTY(QString recipient READ recipient WRITE setRecipient NOTIFY recipientChanged)

public:
    explicit NostrBridge(QObject *parent = nullptr);

    QString secretKey() const;
    QString publicKey() const;
    QString statusText() const;
    QString lastError() const;
    QString lastEventText() const;
    bool connected() const;
    QString recipient() const;
    void setRecipient(const QString &recipient);

    Q_INVOKABLE void addRelay(const QUrl &relayUrl);
    Q_INVOKABLE void removeRelay(const QUrl &relayUrl);
    Q_INVOKABLE void disconnectFromRelays();
    Q_INVOKABLE QString requestEvents(const QStringList &authors,
                                      const QList<int> &kinds,
                                      const QStringList &pTags = QStringList(),
                                      int limit = 10);
    Q_INVOKABLE QString sendMessage(const QString &content,
                                    const QString &recipientBech = QString(),
                                    int kind = 1);

signals:
    void keysChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void lastEventTextChanged();
    void connectionChanged();
    void recipientChanged();
    void eventReceived(const QNostrRelay::Event &event, const QUrl &relay);

private:
    void updateStatus(const QString &text);
    void setError(const QString &message);
    void connectDefaultRelay();
    void subscribeToMentions();

    QString m_secretKey;
    QString m_statusText;
    QString m_lastError;
    QString m_lastEventText;
    QSet<QUrl> m_activeRelays;

    QNostr *m_nostr = nullptr;
    QString m_recipient;
    QString m_publicKeyHex;
    QString m_replySubscriptionId;
    QUrl m_defaultRelay;
};

#endif // NOSTRBRIDGE_H
