#ifndef PUYO_PUYO_WINDOW_H
#define PUYO_PUYO_WINDOW_H

#include <QWidget>
#include "PuyoPuyoGame.h"

class PuyoPuyoWindow : public QWidget
{
    Q_OBJECT

public:
    PuyoPuyoWindow(QWidget *parent = nullptr);
    ~PuyoPuyoWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    PuyoPuyoGame *p_game;
    int timer;
    const int BlockSize = 30;

    void moveBlockD();
    void moveBlockL();
    void moveBlockR();
    bool canMoveD();
    bool canMoveL();
    bool canMoveR();
    void fixBlock();
    void createNewBlock();
    void drawNext();
    void updateBoard();
    void handleGameOver();
};

#endif // PUYO_PUYO_WINDOW_H
