#include "SlimeTrail.h"
#include "ui_SlimeTrail.h"

#include <QDebug>
#include <QMessageBox>
#include <QActionGroup>
#include <QSignalMapper>

SlimeTrail::Player otherPlayer(SlimeTrail::Player player) {
    return (player == SlimeTrail::RedPlayer ?
                    SlimeTrail::BluePlayer : SlimeTrail::RedPlayer);
}

SlimeTrail::SlimeTrail(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::SlimeTrail),
      m_player(SlimeTrail::RedPlayer),
      m_selected(nullptr){

    ui->setupUi(this);

    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(reset()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
    QObject::connect(this, SIGNAL(gameOver(Player)), this, SLOT(showGameOver(Player)));
    QObject::connect(this, SIGNAL(gameOver(Player)), this, SLOT(reset()));

    QSignalMapper* map = new QSignalMapper(this);
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QString holeName = QString("hole%1%2").arg(row).arg(col);
            Hole* hole = this->findChild<Hole*>(holeName);
            Q_ASSERT(hole != nullptr);
            Q_ASSERT(hole->row() == row && hole->col() == col);

            m_board[row][col] = hole;

            int id = row * 8 + col;
            map->setMapping(hole, id);
            QObject::connect(hole, SIGNAL(clicked(bool)), map, SLOT(map()));
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
#else
    QObject::connect(map, SIGNAL(mappedInt(int)), this, SLOT(play(int)));
#endif

    // When the turn ends, switch the player.
    QObject::connect(this, SIGNAL(turnEnded()), this, SLOT(switchPlayer()));

    this->reset();

    this->adjustSize();
    this->setFixedSize(this->size());
}

SlimeTrail::~SlimeTrail() {
    delete ui;
}

Hole* SlimeTrail::holeAt(int row, int col) const {
    if (row >= 0 && row < 8 &&
        col >= 0 && col < 8) {

        return m_board[row][col];
    } else {
        return 0;
    }
}

void SlimeTrail::play(int id) {

    Hole* hole = m_board[id / 8][id % 8];

    if(hole->state() == Hole::EmptyState && hole->isMarked() == true){
        m_prevselected->setState(Hole::BlackState);
        qDebug() << "clicked on: " << hole->objectName();

        if(isGameOver(hole) == 1){
            hole->setState(Hole::WhiteState);
            emit gameOver(SlimeTrail::BluePlayer);
        }
        else if(isGameOver(hole) == 2){
            hole->setState(Hole::WhiteState);
            emit gameOver(SlimeTrail::RedPlayer);
        }
        else if(move(hole) == 1){
            this->reset();
        }
        else{
            m_prevselected = hole;

            emit turnEnded();
        }
    }
}

int SlimeTrail::move(Hole* hole) {

    if (hole->isMarked() == true) {
        this->clearSelectable();
        hole->setState(Hole::WhiteState);

        QList<Hole*> selectable = this->findSelectable(hole);

        if (selectable.count() == 0){
            QMessageBox::information(this, tr("Empate"), tr("O jogo empatou!"));
            //this->reset();
            return 1;
        } else{
            foreach (Hole* tmp, selectable)
                //tmp->setState(Hole::WhiteState);
                tmp->setMarked(true);
                //qDebug() << "entrou aqui";
            m_selected = hole;
        }


    }
    return 0;
}

void SlimeTrail::switchPlayer() {
    // Switch the player.
    m_player = otherPlayer(m_player);

    // Finally, update the status bar.
    this->updateStatusBar();
}

bool isSelectable(Hole* hole) {
    return hole != nullptr &&
            (hole->state() == Hole::EmptyState);
}

void SlimeTrail::clearSelectable() {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Hole* hole = m_board[row][col];
            if (hole->isMarked() == true)
                hole->setMarked(false);
        }
    }
}

QList<Hole*> SlimeTrail::findSelectable(Hole* hole) {
    QList<Hole*> list;

    Hole* left = this->holeAt(hole->row() - 1, hole->col());
    if (isSelectable(left))
        list << left;

    Hole* up = this->holeAt(hole->row(), hole->col() - 1);
    if (isSelectable(up))
        list << up;

    Hole* right = this->holeAt(hole->row() + 1, hole->col());
    if (isSelectable(right))
        list << right;

    Hole* bottom = this->holeAt(hole->row(), hole->col() + 1);
    if (isSelectable(bottom))
        list << bottom;

    Hole* topleft = this->holeAt(hole->row() - 1, hole->col() - 1);
    if (isSelectable(topleft))
        list << topleft;

    Hole* topright = this->holeAt(hole->row() + 1, hole->col() - 1);
    if (isSelectable(topright))
        list << topright;

    Hole* bottomleft = this->holeAt(hole->row() - 1, hole->col() + 1);
    if (isSelectable(bottomleft))
        list << bottomleft;

    Hole* bottomright = this->holeAt(hole->row() + 1, hole->col() + 1);
    if (isSelectable(bottomright))
        list << bottomright;

    return list;
}

int SlimeTrail::isGameOver(Hole* hole) {
    if(hole->row() == 0 && hole->col() == 7){
        return 1;
    }
    else if(hole->row() == 7 && hole->col() == 0){
        return 2;
    }
    return 0;
}

void SlimeTrail::reset() {
    // Reset board.
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Hole* hole = m_board[row][col];
            hole->reset();

            // FIXME: Only neighboor holes should be marked.
            hole->setMarked(false);
        }
    }

    // Mark the starting position.
    m_board[3][4]->setState(Hole::WhiteState);
    m_prevselected = m_board[3][4];

    QList<Hole*> selectable = this->findSelectable(m_board[3][4]);

    foreach (Hole* tmp, selectable)
        tmp->setMarked(true);

    // Reset the player.
    m_player = SlimeTrail::RedPlayer;

    // Finally, update the status bar.
    this->updateStatusBar();
}

void SlimeTrail::showAbout() {
    QMessageBox::information(this, tr("Sobre"), tr("Rastros\n\nFernando Garamvolgyi Mafra Veizaga - fegmve@gmail.com"));
}

void SlimeTrail::showGameOver(Player player) {
    switch (player) {
        case SlimeTrail::RedPlayer:
            QMessageBox::information(this, tr("Vencedor"), tr("Parabéns, o jogador vermelho venceu."));
            break;
        case SlimeTrail::BluePlayer:
            QMessageBox::information(this, tr("Vencedor"), tr("Parabéns, o jogador azul venceu."));
            break;
        default:
            Q_UNREACHABLE();
    }
}

void SlimeTrail::updateStatusBar() {
    QString player(m_player == SlimeTrail::RedPlayer ? "Vermelho" : "Azul");
    ui->statusbar->showMessage(tr("Vez do Jogador %2").arg(player));
}
