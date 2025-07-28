#include "blackjackgame.h"

BlackjackGame::BlackjackGame(const QString &p1, const QString &p2, int s) :
    player1(p1),
    player2(p2),
    stake(s),
    status(Pending)
{
    m_p1.user = player1;
    m_p2.user = player2;
}

void BlackjackGame::start()
{
    m_p1.cards = {drawCard(), drawCard()};
    m_p2.cards = {drawCard(), drawCard()};
    status = Active;
    m_currentPlayer = player1;
    startedAt = QDateTime::currentDateTime();
}

int BlackjackGame::drawCard()
{
    int card = QRandomGenerator::global()->bounded(1, 11); // 1-10
    return card;
}

bool BlackjackGame::isFinished() const
{
    return status == Finished;
}

QString BlackjackGame::getCurrentPlayer() const
{
    return m_currentPlayer;
}

QString BlackjackGame::statusText() const
{
    return QString("Spielstatus:\n%1: %2 Punkte\n%3: %4 Punkte\nAktuell: %5")
           .arg(player1, QString::number(calculatePoints(m_p1.cards)),
                player2, QString::number(calculatePoints(m_p2.cards)),
                m_currentPlayer);
}

QString BlackjackGame::getHandText(const QString &user) const
{
    const PlayerState &ps = (user == player1) ? m_p1 : m_p2;

    QStringList cardStrings;
    for (int c : ps.cards) {
        cardStrings << QString::number(c);
    }

    return QString("%1: %2 Punkte\nKarten: %3")
           .arg(user)
           .arg(calculatePoints(ps.cards))
           .arg(cardStrings.join(", "));
}

bool BlackjackGame::hit(const QString &user)
{
    PlayerState &ps = (user == player1) ? m_p1 : m_p2;
    ps.cards.append(drawCard());

    int points = calculatePoints(ps.cards);
    if (points > 21) {
        ps.busted = true;
        ps.standing = true;
    }

    if (m_p1.standing && m_p2.standing) {
        status = Finished;
        m_winner = determineWinner();
    } else {
        m_currentPlayer = (m_currentPlayer == player1) ? player2 : player1;
    }

    return ps.busted;
}

void BlackjackGame::stand(const QString &user)
{
    PlayerState &ps = (user == player1) ? m_p1 : m_p2;
    ps.standing = true;

    if (m_p1.standing && m_p2.standing) {
        status = Finished;
        m_winner = determineWinner();
    } else {
        m_currentPlayer = (m_currentPlayer == player1) ? player2 : player1;
    }
}

int BlackjackGame::calculatePoints(const QList<int> &cards) const
{
    int sum = 0;
    for (int c : cards) {
        sum += c;
    }
    return sum;
}

QString BlackjackGame::determineWinner() const
{
    int p1 = m_p1.busted ? 0 : calculatePoints(m_p1.cards);
    int p2 = m_p2.busted ? 0 : calculatePoints(m_p2.cards);

    if (p1 > p2) {
        return player1;
    }
    if (p2 > p1) {
        return player2;
    }
    return "Unentschieden";
}

const QList<int> &BlackjackGame::getCards(const QString &user) const
{
    if (user == player1) {
        return m_p1.cards;
    } else {
        return m_p2.cards;
    }
}
