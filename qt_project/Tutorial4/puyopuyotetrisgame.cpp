#include "PuyoPuyoTetrisGame.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <QTimer>
#include <stack>
using namespace std;

PuyoPuyoTetrisGame::PuyoPuyoTetrisGame()
{
    ROW = 22;
    COL = 10;
    axis_row = 0;
    axis_col = COL / 2;
    score = 0;
    gameover = false;
    board.resize(ROW, vector<string>(COL, "Empty"));
    boardColors.resize(ROW, vector<QColor>(COL, QColor(255, 255, 255)));
    srand(time(0));

    puyoShapes = {
        // Puyo shape
        {{0, 1, 0},
         {0, 1, 0},
         {0, 0, 0}}
    };

    tetrisShapes = {
        // I shape
        {{0, 0, 0, 0},
         {1, 1, 1, 1},
         {0, 0, 0, 0},
         {0, 0, 0, 0}},
        // O shape
        {{1, 1},
         {1, 1}},
        // T shape
        {{0, 1, 0},
         {1, 1, 1},
         {0, 0, 0}},
        // S shape
        {{0, 1, 1},
         {1, 1, 0},
         {0, 0, 0}},
        // Z shape
        {{1, 1, 0},
         {0, 1, 1},
         {0, 0, 0}},
        // J shape
        {{1, 0, 0},
         {1, 1, 1},
         {0, 0, 0}},
        // L shape
        {{0, 0, 1},
         {1, 1, 1},
         {0, 0, 0}}
    };

    puyoColors = {
        QColor(255, 0, 0),   // Red
        QColor(255, 255, 0), // Yellow
        QColor(0, 255, 0),   // Green
        QColor(0, 0, 255),   // Blue
        QColor(128, 0, 128)  // Purple
    };

    tetrisColors = {
        QColor(0, 255, 255), // I shape - Cyan
        QColor(255, 255, 0), // O shape - Yellow
        QColor(128, 0, 128), // T shape - Purple
        QColor(0, 255, 0),   // S shape - Green
        QColor(255, 0, 0),   // Z shape - Red
        QColor(0, 0, 255),   // J shape - Blue
        QColor(255, 165, 0)  // L shape - Orange
    };

    initializeBoard();
    generateNextShapes();
    spawnNewShape();
}

PuyoPuyoTetrisGame::~PuyoPuyoTetrisGame()
{
}

void PuyoPuyoTetrisGame::initializeBoard()
{
    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            board[i][j] = "Empty";
            if (i < 2) {
                boardColors[i][j] = QColor(192, 192, 192); // 첫 두 줄은 회색
            } else {
                boardColors[i][j] = QColor(255, 255, 255); // 나머지는 흰색
            }
        }
    }
}

vector<vector<int>> PuyoPuyoTetrisGame::getRandomShape()
{
    if (isPuyo) {
        return puyoShapes[0];
    } else {
        return tetrisShapes[rand() % tetrisShapes.size()];
    }
}

vector<vector<QColor>> PuyoPuyoTetrisGame::getRandomColors()
{
    if (isPuyo) {
        vector<vector<QColor>> colors(3, vector<QColor>(3, QColor(255, 255, 255)));
        colors[1][1] = puyoColors[rand() % puyoColors.size()];
        colors[2][1] = puyoColors[rand() % puyoColors.size()];
        return colors;
    } else {
        vector<vector<QColor>> colors(4, vector<QColor>(4, QColor(255, 255, 255)));
        QColor color = tetrisColors[rand() % tetrisColors.size()];
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (i < tetrisShapes[0].size() && j < tetrisShapes[0][0].size() && tetrisShapes[0][i][j] == 1)
                    colors[i][j] = color;
        return colors;
    }
}

void PuyoPuyoTetrisGame::fixBlock()
{
    for (int i = 0; i < currentShape.size(); ++i)
    {
        for (int j = 0; j < currentShape[i].size(); ++j)
        {
            if (currentShape[i][j] == 1)
            {
                int row = axis_row + i;
                int col = axis_col + j;
                board[row][col] = "FixedBlock";
                boardColors[row][col] = darkenColor(currentShapeColors[i][j]);
            }
        }
    }

    if (checkGameOver())
    {
        gameover = true;
        emit gameOver();
    }
    else
    {
        clearConnectedTiles();
        applyGravity();
    }
}

QColor PuyoPuyoTetrisGame::darkenColor(const QColor& color)
{
    int factor = 90;
    return QColor(max(color.red() - factor, 0),
                  max(color.green() - factor, 0),
                  max(color.blue() - factor, 0));
}

QColor PuyoPuyoTetrisGame::brightenColor(const QColor& color)
{
    int factor = 140;
    return QColor(min(color.red() + factor, 255),
                  min(color.green() + factor, 255),
                  min(color.blue() + factor, 255));
}

void PuyoPuyoTetrisGame::clearConnectedTiles()
{
    bool tilesCleared = false;
    vector<vector<bool>> visited(ROW, vector<bool>(COL, false));
    vector<vector<int>> directions = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    vector<pair<int, int>> connected;

    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if (board[i][j] == "FixedBlock" && !visited[i][j])
            {
                connected.clear();
                stack<pair<int, int>> stack;
                stack.push({i, j});
                QColor color = boardColors[i][j];

                while (!stack.empty())
                {
                    auto [x, y] = stack.top();
                    stack.pop();
                    if (x < 0 || x >= ROW || y < 0 || y >= COL || visited[x][y] || board[x][y] != "FixedBlock" || boardColors[x][y] != color)
                    {
                        continue;
                    }

                    visited[x][y] = true;
                    connected.push_back({x, y});

                    for (auto& dir : directions)
                    {
                        stack.push({x + dir[0], y + dir[1]});
                    }
                }

                if (connected.size() >= 4)
                {
                    for (auto& pos : connected)
                    {
                        boardColors[pos.first][pos.second] = brightenColor(boardColors[pos.first][pos.second]);
                    }
                    tilesCleared = true;

                    // Delay to show brightened tiles before clearing them
                    QTimer::singleShot(200, [this, connected, color]() {
                        for (auto& pos : connected)
                        {
                            board[pos.first][pos.second] = "Empty";
                            boardColors[pos.first][pos.second] = QColor(255, 255, 255);
                        }

                        applyGravity();
                        score += connected.size();

                    });
                }
            }
        }
    }
}

bool PuyoPuyoTetrisGame::checkGameOver()
{
    for (int j = 0; j < COL; ++j)
    {
        if (board[1][j] != "Empty")
        {
            return true;
        }
    }
    return false;
}

vector<vector<QColor>> PuyoPuyoTetrisGame::getCurrentShapeColors()
{
    return currentShapeColors;
}

vector<vector<int>> PuyoPuyoTetrisGame::getCurrentShape()
{
    return currentShape;
}

void PuyoPuyoTetrisGame::rotateCurrentShapeClockwise()
{
    auto rotatedShape = rotateShape(currentShape, true);
    auto rotatedColors = rotateColors(currentShapeColors, true);
    if (canRotate(rotatedShape))
    {
        currentShape = rotatedShape;
        currentShapeColors = rotatedColors;
    }
}

void PuyoPuyoTetrisGame::rotateCurrentShapeCounterClockwise()
{
    auto rotatedShape = rotateShape(currentShape, false);
    auto rotatedColors = rotateColors(currentShapeColors, false);
    if (canRotate(rotatedShape))
    {
        currentShape = rotatedShape;
        currentShapeColors = rotatedColors;
    }
}

vector<vector<int>> PuyoPuyoTetrisGame::rotateShape(const vector<vector<int>>& shape, bool clockwise)
{
    int rows = shape.size();
    int cols = shape[0].size();
    vector<vector<int>> rotated(cols, vector<int>(rows, 0));

    if (clockwise)
    {
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                rotated[j][rows - 1 - i] = shape[i][j];
            }
        }
    }
    else
    {
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                rotated[cols - 1 - j][i] = shape[i][j];
            }
        }
    }

    return rotated;
}

vector<vector<QColor>> PuyoPuyoTetrisGame::rotateColors(const vector<vector<QColor>>& colors, bool clockwise)
{
    int rows = colors.size();
    int cols = colors[0].size();
    vector<vector<QColor>> rotated(cols, vector<QColor>(rows));

    if (clockwise)
    {
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                rotated[j][rows - 1 - i] = colors[i][j];
            }
        }
    }
    else
    {
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                rotated[cols - 1 - j][i] = colors[i][j];
            }
        }
    }

    return rotated;
}

bool PuyoPuyoTetrisGame::canRotate(const vector<vector<int>>& rotatedShape)
{
    int n = rotatedShape.size();
    int m = rotatedShape[0].size();
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
        {
            if (rotatedShape[i][j] == 1)
            {
                int newRow = axis_row + i;
                int newCol = axis_col + j;
                if (newRow < 0 || newRow >= ROW || newCol < 0 || newCol >= COL || board[newRow][newCol] != "Empty")
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool PuyoPuyoTetrisGame::canMoveDown()
{
    int next_row = axis_row + 1;
    for (int i = 0; i < currentShape.size(); ++i)
    {
        for (int j = 0; j < currentShape[i].size(); ++j)
        {
            if (currentShape[i][j] == 1)
            {
                int row = next_row + i;
                int col = axis_col + j;
                if (row >= ROW || board[row][col] != "Empty")
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void PuyoPuyoTetrisGame::dropBlock()
{
    while (canMoveDown())
    {
        axis_row++;
    }
    fixBlock();
    spawnNewShape();
}

void PuyoPuyoTetrisGame::generateNextShapes()
{
    while (nextQueue.size() < 2)
    {
        nextQueue.push({getRandomShape(), getRandomColors()});
    }
}

void PuyoPuyoTetrisGame::spawnNewShape()
{
    axis_row = 0;
    axis_col = COL / 2;
    isPuyo = rand() % 2 == 0;
    auto nextShape = nextQueue.front();
    currentShape = nextShape.first;
    currentShapeColors = nextShape.second;
    nextQueue.pop();
    generateNextShapes();
}

void PuyoPuyoTetrisGame::applyGravity()
{
    for (int col = 0; col < COL; ++col)
    {
        int empty_row = ROW - 1;
        for (int row = ROW - 1; row >= 0; --row)
        {
            if (board[row][col] != "Empty")
            {
                swap(board[row][col], board[empty_row][col]);
                swap(boardColors[row][col], boardColors[empty_row][col]);
                --empty_row;
            }
        }
    }
}
