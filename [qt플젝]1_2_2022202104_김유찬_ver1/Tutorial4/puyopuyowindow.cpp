#include "PuyoPuyoWindow.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTimerEvent>
#include <QMessageBox>

PuyoPuyoWindow::PuyoPuyoWindow(QWidget *parent)
    : QWidget(parent), p_game(new PuyoPuyoGame())
{
    connect(p_game, &PuyoPuyoGame::tilesCleared, this, &PuyoPuyoWindow::updateBoard);
    connect(p_game, &PuyoPuyoGame::gameOver, this, &PuyoPuyoWindow::handleGameOver);
    timer = startTimer(500); // 타이머 시작
    resize(700, 1100);
    setWindowTitle("Puyo Puyo");
}

PuyoPuyoWindow::~PuyoPuyoWindow()
{
    delete p_game;
}

void PuyoPuyoWindow::paintEvent(QPaintEvent *event)
{
    int ROW = p_game->ROW;  // 게임 보드의 행 수
    int COL = p_game->COL;  // 게임 보드의 열 수
    auto& board = p_game->board;  // 게임 보드 상태
    auto& boardColors = p_game->boardColors;  // 게임 보드의 색상
    double axis_row = p_game->axis_row;  // 현재 블록의 행 위치
    double axis_col = p_game->axis_col;  // 현재 블록의 열 위치
    int score = p_game->score;  // 현재 점수

    QPainter painter(this);  // QPainter 객체 생성
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));  // 흰색 브러시 설정
    painter.drawRect(50, 50, COL * BlockSize, ROW * BlockSize);  // 게임 보드 영역 그리기

    painter.setPen(Qt::black);  // 검정색 펜 설정
    painter.setFont(QFont("Arial", 14));  // 폰트 설정

    painter.drawText(QRect(50 + COL * BlockSize + 20, 50, 100, 20), Qt::AlignCenter, "NEXT");  // "NEXT" 텍스트 그리기
    for (int i = 0; i < ROW; i++)  // 보드의 각 행 순회
    {
        for (int j = 0; j < COL; j++)  // 보드의 각 열 순회
        {
            if (i < 2)  // 첫 두 줄인 경우
            {
                painter.setBrush(QBrush(QColor(192, 192, 192), Qt::SolidPattern));  // 회색 브러시 설정
            }
            else
            {
                painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));  // 흰색 브러시 설정
            }
            painter.drawRect(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);  // 보드 칸 그리기

            if (board[i][j] == "FixedBlock")  // 고정된 블록인 경우
            {
                painter.setBrush(QBrush(boardColors[i][j], Qt::SolidPattern));  // 블록 색상 설정
                painter.drawEllipse(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);  // 블록 그리기
            }
        }
    }

    auto shape = p_game->getCurrentShape();  // 현재 블록 모양 가져오기
    auto shapeColors = p_game->getCurrentShapeColors();  // 현재 블록 색상 가져오기
    for (int i = 0; i < shape.size(); ++i)  // 블록의 각 행 순회
    {
        for (int j = 0; j < shape[i].size(); ++j)  // 블록의 각 열 순회
        {
            if (shape[i][j] == 1)  // 블록이 있는 부분인 경우
            {
                int row = axis_row + i;  // 블록의 행 위치 계산
                int col = axis_col + j;  // 블록의 열 위치 계산
                painter.setBrush(QBrush(shapeColors[i][j], Qt::SolidPattern));  // 블록 색상 설정
                painter.drawEllipse(col * BlockSize + 50, row * BlockSize + 50, BlockSize, BlockSize);  // 블록 그리기
            }
        }
    }

    int next_shape_y = 50 + 100;  // 다음 블록 표시 시작 위치
    std::queue<std::pair<std::vector<std::vector<int>>, std::vector<std::vector<QColor>>>> tempQueue = p_game->nextQueue;  // 다음 블록 큐 복사
    int count = 0;  // 다음 블록 카운트
    while (!tempQueue.empty() && count < 2)  // 다음 블록이 있고, 2개 미만인 경우
    {
        auto nextShape = tempQueue.front();  // 다음 블록 가져오기
        tempQueue.pop();  // 큐에서 제거
        auto next_shape = nextShape.first;  // 다음 블록 모양
        auto nextColors = nextShape.second;  // 다음 블록 색상
        for (int i = 0; i < next_shape.size(); ++i)  // 다음 블록의 각 행 순회
        {
            for (int j = 0; j < next_shape[i].size(); ++j)  // 다음 블록의 각 열 순회
            {
                if (next_shape[i][j] == 1)  // 블록이 있는 부분인 경우
                {
                    painter.setBrush(QBrush(nextColors[i][j], Qt::SolidPattern));  // 블록 색상 설정
                    painter.drawEllipse(COL * BlockSize + 70 + j * BlockSize, next_shape_y + i * BlockSize, BlockSize, BlockSize);  // 블록 그리기
                }
            }
        }
        next_shape_y += (next_shape.size() + 1) * BlockSize;  // 다음 블록 위치 계산
        count++;  // 다음 블록 카운트 증가
    }

    painter.setPen(Qt::black);  // 검정색 펜 설정
    painter.setFont(QFont("Arial", 14));  // 폰트 설정
    painter.drawText(QRect(50 + COL * BlockSize + 20, ROW * BlockSize + 80, 150, 20), Qt::AlignCenter, "current score : " + QString::number(score));  // 현재 점수 표시
}

void PuyoPuyoWindow::keyPressEvent(QKeyEvent *event)
{
    if (p_game->gameover)
        return;

    switch (event->key())
    {
    case Qt::Key_Down:
        moveBlockD();
        break;
    case Qt::Key_Left:
        moveBlockL();
        break;
    case Qt::Key_Right:
        moveBlockR();
        break;
    case Qt::Key_Z:
        p_game->rotateCurrentShapeCounterClockwise();
        drawNext();
        break;
    case Qt::Key_X:
        p_game->rotateCurrentShapeClockwise();
        drawNext();
        break;
    case Qt::Key_Space:
        p_game->dropBlock();
        drawNext();
        break;
    default:
        break;
    }
}

void PuyoPuyoWindow::timerEvent(QTimerEvent *event)
{
    if (p_game->gameover)
        return;

    if (event->timerId() == timer)
    {
        moveBlockD();
        drawNext();
    }
}

void PuyoPuyoWindow::moveBlockD()
{
    if (canMoveD())
    {
        p_game->axis_row++;
    }
    else
    {
        p_game->fixBlock();
        p_game->clearConnectedTiles();
        createNewBlock();
    }
    drawNext();
}

void PuyoPuyoWindow::moveBlockL()
{
    if (canMoveL())
    {
        p_game->axis_col--;
        drawNext();
    }
}

void PuyoPuyoWindow::moveBlockR()
{
    if (canMoveR())
    {
        p_game->axis_col++;
        drawNext();
    }
}

bool PuyoPuyoWindow::canMoveD()
{
    int next_row = p_game->axis_row + 1;  // 다음 행 위치 계산

    auto shape = p_game->getCurrentShape();  // 현재 블록 모양 가져오기
    for (int i = 0; i < shape.size(); ++i)  // 블록의 각 행 순회
    {
        for (int j = 0; j < shape[i].size(); ++j)  // 블록의 각 열 순회
        {
            if (shape[i][j] == 1)  // 블록이 있는 부분에 대해 체크
            {
                int row = next_row + i;  // 다음 행 위치 계산
                int col = p_game->axis_col + j;  // 현재 열 위치 계산
                if (row >= p_game->ROW || p_game->board[row][col] != "Empty")  // 경계를 넘거나 비어있지 않은 경우
                {
                    return false;  // 이동 불가능
                }
            }
        }
    }
    return true;  // 이동 가능
}



bool PuyoPuyoWindow::canMoveL()
{
    int next_col = p_game->axis_col - 1;

    auto shape = p_game->getCurrentShape();
    for (int i = 0; i < shape.size(); ++i)
    {
        for (int j = 0; j < shape[i].size(); ++j)
        {
            if (shape[i][j] == 1)
            {
                int row = p_game->axis_row + i;
                int col = next_col + j;
                if (col < 0 || p_game->board[row][col] != "Empty")
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool PuyoPuyoWindow::canMoveR()
{
    int next_col = p_game->axis_col + 1;  // 다음 열 위치 계산

    auto shape = p_game->getCurrentShape();  // 현재 블록 모양 가져오기
    for (int i = 0; i < shape.size(); ++i)
    {
        for (int j = 0; j < shape[i].size(); ++j)
        {
            if (shape[i][j] == 1)  // 블록이 있는 부분에 대해 체크
            {
                int row = p_game->axis_row + i;  // 현재 블록의 행 위치 계산
                int col = next_col + j;  // 다음 열 위치 계산
                if (col >= p_game->COL || p_game->board[row][col] != "Empty")  // 경계를 넘거나 비어있지 않은 경우
                {
                    return false;  // 이동 불가능
                }
            }
        }
    }
    return true;  // 이동 가능
}

void PuyoPuyoWindow::fixBlock()
{
    p_game->fixBlock();
}

void PuyoPuyoWindow::createNewBlock()
{
    p_game->spawnNewShape();
}

void PuyoPuyoWindow::drawNext()
{
    if (p_game->gameover)
    {
        for (int i = 0; i < p_game->ROW; i++)
            for (int j = 0; j < p_game->COL; j++)
            {
                if (p_game->board[i][j] == "Empty")
                    p_game->board[i][j] = "FixedBlock";
            }
        return;
    }

    update();
}

void PuyoPuyoWindow::updateBoard()
{
    update();
}

void PuyoPuyoWindow::handleGameOver()
{
    killTimer(timer);
    QMessageBox::information(this, "Game Over", "Game Over! Your score: " + QString::number(p_game->score));
}
