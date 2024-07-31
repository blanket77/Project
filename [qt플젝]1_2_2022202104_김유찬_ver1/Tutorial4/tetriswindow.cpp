#include "TetrisWindow.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTimerEvent>
#include <QMessageBox>

TetrisWindow::TetrisWindow(QWidget *parent)
    : QWidget(parent), p_game(new TetrisGame())
{
    connect(p_game, &TetrisGame::linesCleared, this, &TetrisWindow::updateBoard);
    connect(p_game, &TetrisGame::gameOver, this, &TetrisWindow::handleGameOver);
    timer = startTimer(500); // 타이머 시작
    resize(700, 1100);
    setWindowTitle("Tetris");
}

TetrisWindow::~TetrisWindow()
{
    delete p_game; // 게임 객체 삭제
}

void TetrisWindow::paintEvent(QPaintEvent *event)
{
    int ROW = p_game->ROW; // 게임 보드의 행 수
    int COL = p_game->COL; // 게임 보드의 열 수
    auto& board = p_game->board; // 현재 게임 보드
    auto& boardColors = p_game->boardColors; // 현재 게임 보드의 색상
    double axis_row = p_game->axis_row; // 현재 축의 행 위치
    double axis_col = p_game->axis_col; // 현재 축의 열 위치
    int score = p_game->score; // 현재 점수

    QPainter painter(this); // QPainter 객체 생성
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern)); // 브러시를 하얀색으로 설정
    painter.drawRect(50, 50, COL * BlockSize, ROW * BlockSize); // 게임 보드 그리기

    painter.setPen(Qt::black); // 펜을 검은색으로 설정
    painter.setFont(QFont("Arial", 14)); // 폰트를 Arial, 크기 14로 설정

    painter.drawText(QRect(50 + COL * BlockSize + 20, 50, 100, 20), Qt::AlignCenter, "NEXT"); // "NEXT" 텍스트 그리기
    // 보드 그리기
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            painter.setBrush(QBrush(boardColors[i][j], Qt::SolidPattern));
            painter.drawRect(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);
            if (board[i][j] == "FixedBlock")
            {
                painter.setBrush(QBrush(boardColors[i][j], Qt::SolidPattern));
                painter.drawRect(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);
            }
        }
    }

    // 현재 블록 그리기
    auto shape = p_game->getCurrentShape();
    QColor shapeColor = p_game->shapeColors[p_game->currentShapeIndex];
    painter.setBrush(QBrush(shapeColor, Qt::SolidPattern));
    for (int i = 0; i < shape.size(); ++i)
    {
        for (int j = 0; j < shape[i].size(); ++j)
        {
            if (shape[i][j] == 1)
            {
                int row = axis_row + i;
                int col = axis_col + j;
                painter.drawRect(col * BlockSize + 50, row * BlockSize + 50, BlockSize, BlockSize);
            }
        }
    }

    // 다음 블록 그리기
    int next_shape_y = 50 + 100;
    queue<int> tempQueue = p_game->nextQueue; // 복사본 사용
    while (!tempQueue.empty())
    {
        int shapeIndex = tempQueue.front();
        tempQueue.pop();
        auto next_shape = p_game->shapes[shapeIndex];
        QColor nextShapeColor = p_game->shapeColors[shapeIndex];
        painter.setBrush(QBrush(nextShapeColor, Qt::SolidPattern));
        for (int i = 0; i < next_shape.size(); ++i)
        {
            for (int j = 0; j < next_shape[i].size(); ++j)
            {
                if (next_shape[i][j] == 1)
                {
                    painter.drawRect(COL * BlockSize + 70 + j * BlockSize, next_shape_y + i * BlockSize, BlockSize, BlockSize);
                }
            }
        }
        next_shape_y += (next_shape.size() + 1) * BlockSize;
    }

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(QRect(50 + COL * BlockSize + 20, ROW * BlockSize + 80, 150, 20),Qt::AlignCenter, "current line : " + QString::number(score));
}

//키를 눌렀을 때
void TetrisWindow::keyPressEvent(QKeyEvent *event)
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

//시간이 흐른다.
void TetrisWindow::timerEvent(QTimerEvent *event)
{
    if (p_game->gameover)
        return;

    if (event->timerId() == timer)
    {
        moveBlockD();
        drawNext();
    }
}

void TetrisWindow::moveBlockD()
{
    if (canMoveD())
    {
        p_game->axis_row++;
    }
    else
    {
        p_game->fixBlock();
        p_game->clearFullLines();
        createNewBlock();
    }
    drawNext();
}

void TetrisWindow::moveBlockL()
{
    if (canMoveL())
    {
        p_game->axis_col--;
        drawNext();
    }
}

void TetrisWindow::moveBlockR()
{
    if (canMoveR())
    {
        p_game->axis_col++;
        drawNext();
    }
}

bool TetrisWindow::canMoveD()
{
    int next_row = p_game->axis_row + 1;

    auto shape = p_game->getCurrentShape();
    for (int i = 0; i < shape.size(); ++i)
    {
        for (int j = 0; j < shape[i].size(); ++j)
        {
            if (shape[i][j] == 1)
            {
                int row = next_row + i;
                int col = p_game->axis_col + j;
                if (row >= p_game->ROW || p_game->board[row][col] != "Empty")
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool TetrisWindow::canMoveL()
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

bool TetrisWindow::canMoveR()
{
    int next_col = p_game->axis_col + 1;

    auto shape = p_game->getCurrentShape();
    for (int i = 0; i < shape.size(); ++i)
    {
        for (int j = 0; j < shape[i].size(); ++j)
        {
            if (shape[i][j] == 1)
            {
                int row = p_game->axis_row + i;
                int col = next_col + j;
                if (col >= p_game->COL || p_game->board[row][col] != "Empty")
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void TetrisWindow::rotateBlockClockwise()
{
    p_game->rotateCurrentShapeClockwise();
    drawNext();
}

void TetrisWindow::rotateBlockCounterClockwise()
{
    p_game->rotateCurrentShapeCounterClockwise();
    drawNext();
}

void TetrisWindow::fixBlock()
{
    p_game->fixBlock();
}

void TetrisWindow::createNewBlock()
{
    p_game->spawnNewShape();
}

void TetrisWindow::drawNext()
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

void TetrisWindow::updateBoard()
{
    update();
}

void TetrisWindow::handleGameOver()
{
    killTimer(timer);
    QMessageBox::information(this, "Game Over", "Game Over! Your score: " + QString::number(p_game->score));
}
