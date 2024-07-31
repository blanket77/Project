#include "PuyoPuyoTetrisWindow.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTimerEvent>
#include <QMessageBox>

PuyoPuyoTetrisWindow::PuyoPuyoTetrisWindow(QWidget *parent)
    : QWidget(parent), p_game(new PuyoPuyoTetrisGame())
{
    connect(p_game, &PuyoPuyoTetrisGame::tilesCleared, this, &PuyoPuyoTetrisWindow::updateBoard);
    connect(p_game, &PuyoPuyoTetrisGame::gameOver, this, &PuyoPuyoTetrisWindow::handleGameOver);
    timer = startTimer(500); // 타이머 시작
    resize(700, 1100);
    setWindowTitle("Puyo Puyo Tetris");
}

PuyoPuyoTetrisWindow::~PuyoPuyoTetrisWindow()
{
    delete p_game;
}

void PuyoPuyoTetrisWindow::paintEvent(QPaintEvent *event)
{
    int ROW = p_game->ROW;
    int COL = p_game->COL;
    auto& board = p_game->board;
    auto& boardColors = p_game->boardColors;
    double axis_row = p_game->axis_row;
    double axis_col = p_game->axis_col;
    int score = p_game->score;

    QPainter painter(this);
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
    painter.drawRect(50, 50, COL * BlockSize, ROW * BlockSize);

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14));

    painter.drawText(QRect(50 + COL * BlockSize + 20, 50, 100, 20), Qt::AlignCenter, "NEXT");
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            if (i < 2)
            {
                painter.setBrush(QBrush(QColor(192, 192, 192), Qt::SolidPattern));
            }
            else
            {
                painter.setBrush(QBrush(Qt::white, Qt::SolidPattern)); // 바탕을 흰색으로 초기화
            }
            painter.drawRect(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);

            if (board[i][j] == "FixedBlock")
            {
                if (p_game->isPuyo)
                {
                    painter.setBrush(QBrush(boardColors[i][j], Qt::SolidPattern));
                    painter.drawEllipse(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);
                }
                else
                {
                    painter.setBrush(QBrush(boardColors[i][j], Qt::SolidPattern));
                    painter.drawRect(j * BlockSize + 50, i * BlockSize + 50, BlockSize, BlockSize);
                }
            }
        }
    }

    auto shape = p_game->getCurrentShape();
    auto shapeColors = p_game->getCurrentShapeColors();
    for (int i = 0; i < shape.size(); ++i)
    {
        for (int j = 0; j < shape[i].size(); ++j)
        {
            if (shape[i][j] == 1)
            {
                int row = axis_row + i;
                int col = axis_col + j;
                if (p_game->isPuyo)
                {
                    painter.setBrush(QBrush(shapeColors[i][j], Qt::SolidPattern));
                    painter.drawEllipse(col * BlockSize + 50, row * BlockSize + 50, BlockSize, BlockSize);
                }
                else
                {
                    painter.setBrush(QBrush(shapeColors[i][j], Qt::SolidPattern));
                    painter.drawRect(col * BlockSize + 50, row * BlockSize + 50, BlockSize, BlockSize);
                }
            }
        }
    }

    int next_shape_y = 50 + 100;
    std::queue<std::pair<std::vector<std::vector<int>>, std::vector<std::vector<QColor>>>> tempQueue = p_game->nextQueue;
    int count = 0;
    while (!tempQueue.empty() && count < 2)
    {
        auto nextShape = tempQueue.front();
        tempQueue.pop();
        auto next_shape = nextShape.first;
        auto nextColors = nextShape.second;
        for (int i = 0; i < next_shape.size(); ++i)
        {
            for (int j = 0; j < next_shape[i].size(); ++j)
            {
                if (next_shape[i][j] == 1)
                {
                    if (p_game->isPuyo)
                    {
                        painter.setBrush(QBrush(nextColors[i][j], Qt::SolidPattern));
                        painter.drawEllipse(COL * BlockSize + 70 + j * BlockSize, next_shape_y + i * BlockSize, BlockSize, BlockSize);
                    }
                    else
                    {
                        painter.setBrush(QBrush(nextColors[i][j], Qt::SolidPattern));
                        painter.drawRect(COL * BlockSize + 70 + j * BlockSize, next_shape_y + i * BlockSize, BlockSize, BlockSize);
                    }
                }
            }
        }
        next_shape_y += (next_shape.size() + 1) * BlockSize;
        count++;
    }

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(QRect(50 + COL * BlockSize + 20, ROW * BlockSize + 80, 150, 20), Qt::AlignCenter, "current line : " + QString::number(score));
}

void PuyoPuyoTetrisWindow::keyPressEvent(QKeyEvent *event)
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

void PuyoPuyoTetrisWindow::timerEvent(QTimerEvent *event)
{
    if (p_game->gameover)
        return;

    if (event->timerId() == timer)
    {
        moveBlockD();
        drawNext();
    }
}

void PuyoPuyoTetrisWindow::moveBlockD()
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

void PuyoPuyoTetrisWindow::moveBlockL()
{
    if (canMoveL())
    {
        p_game->axis_col--;
        drawNext();
    }
}

void PuyoPuyoTetrisWindow::moveBlockR()
{
    if (canMoveR())
    {
        p_game->axis_col++;
        drawNext();
    }
}

bool PuyoPuyoTetrisWindow::canMoveD()
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

bool PuyoPuyoTetrisWindow::canMoveL()
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

bool PuyoPuyoTetrisWindow::canMoveR()
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

void PuyoPuyoTetrisWindow::fixBlock()
{
    p_game->fixBlock();
}

void PuyoPuyoTetrisWindow::createNewBlock()
{
    p_game->spawnNewShape();
}

void PuyoPuyoTetrisWindow::drawNext()
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

void PuyoPuyoTetrisWindow::updateBoard()
{
    update();
}

void PuyoPuyoTetrisWindow::handleGameOver()
{
    killTimer(timer);
    QMessageBox::information(this, "Game Over", "Game Over! Your score: " + QString::number(p_game->score));
}
