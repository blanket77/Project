#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include <QWidget>
#include "TetrisGame.h"

class TetrisWindow : public QWidget
{
    Q_OBJECT

public:
    TetrisWindow(QWidget *parent = nullptr);
    ~TetrisWindow();

protected:
    void paintEvent(QPaintEvent *event) override; // 화면을 다시 그리는 이벤트
    void keyPressEvent(QKeyEvent *event) override; //키가 눌렀을 때의 이벤트
    void timerEvent(QTimerEvent *event) override; // 타이머 이벤트

private slots:
    void updateBoard();// 보드를 업데이트하는 슬롯
    void handleGameOver(); //게임 오버 처리를 하는 슬롯

private:
    TetrisGame *p_game; // TetrisGame 객체 포인터
    int timer; //타이머 ID
    const int BlockSize = 30; // 블록 크기

    void moveBlockD(); // 블록 아래로
    void moveBlockL(); // 블록 왼쪽으로
    void moveBlockR(); // 블록 오른쪽으로
    bool canMoveD(); // 블록 아래로 가능?
    bool canMoveL(); // 블록 왼쪽으로 가능?
    bool canMoveR(); // 블록 오른쪽으로 가능?
    void rotateBlockClockwise(); //블록 시계 방향으로 회전
    void rotateBlockCounterClockwise(); //블록을 반시계 방향으로 회전
    void fixBlock(); // 현재 블록을 보드에 고정
    void createNewBlock(); // 새로운 블록을 만든다.
    void drawNext(); // 다음 블록을 그림
};

#endif // TETRISWINDOW_H
