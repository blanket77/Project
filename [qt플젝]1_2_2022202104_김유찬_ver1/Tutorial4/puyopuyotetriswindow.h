#ifndef PUYO_PUYO_TETRIS_WINDOW_H
#define PUYO_PUYO_TETRIS_WINDOW_H

#include <QWidget>
#include "PuyoPuyoTetrisGame.h"

class PuyoPuyoTetrisWindow : public QWidget
{
    Q_OBJECT

public:
    PuyoPuyoTetrisWindow(QWidget *parent = nullptr);
    ~PuyoPuyoTetrisWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    PuyoPuyoTetrisGame *p_game;
    int timer;
    const int BlockSize = 30;

    void moveBlockD();
    void moveBlockL();
    void moveBlockR();
    void rotateBlockClockwise();
    void rotateBlockCounterClockwise();
    void fixBlock();
    void createNewBlock();
    void drawNext();
    void updateBoard();
    void handleGameOver();

    bool canMoveD();
    bool canMoveL();
    bool canMoveR();
};

#endif // PUYO_PUYO_TETRIS_WINDOW_H
