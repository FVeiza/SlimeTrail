#ifndef SLIMETRAIL_H
#define SLIMETRAIL_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class SlimeTrail;
}
QT_END_NAMESPACE

class Hole;

class SlimeTrail : public QMainWindow {
    Q_OBJECT

public:
    enum Player {
        RedPlayer,
        BluePlayer
    };
    Q_ENUM(Player)

    SlimeTrail(QWidget *parent = nullptr);
    virtual ~SlimeTrail();

    Hole* holeAt(int row, int col) const;

signals:
    void turnEnded();
    void gameOver(Player player);

private:
    Ui::SlimeTrail *ui;
    Player m_player;
    Hole* m_board[8][8];
    Hole* m_selected;
    Hole* m_prevselected;

    int isGameOver(Hole* hole);

private slots:
    void play(int id);
    void switchPlayer();
    void reset();
    QList<Hole*> findSelectable(Hole* hole);
    int move(Hole* hole);
    void clearSelectable();

    void showAbout();
    void showGameOver(Player player);
    void updateStatusBar();

};

#endif // SLIMETRAIL_H
