#ifndef BLACKJACKGAME_H
#define BLACKJACKGAME_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMap>
#include <QRandomGenerator>

struct PlayerState {
    QString user;
    QList<int> cards;
    bool standing = false;
    bool busted = false;
};

class BlackjackGame
{
public:
    BlackjackGame(const QString &player1, const QString &player2, int stake);

    enum GameStatus {
        Pending,
        Active,
        Finished
    };

    void start();
    bool isFinished() const;
    QString getCurrentPlayer() const;
    QString statusText() const;

    bool hit(const QString &user);
    void stand(const QString &user);

    QString getHandText(const QString &user) const;
    int calculatePoints(const QList<int> &cards) const;
    QString determineWinner() const;

    const QList<int> &getCards(const QString &user) const;

    QString player1;
    QString player2;
    int stake;
    QDateTime startedAt;
    GameStatus status;

private:
    PlayerState m_p1;
    PlayerState m_p2;
    QString m_currentPlayer;
    QString m_winner;

    int drawCard();
};

#endif // BLACKJACKGAME_H
