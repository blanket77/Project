#include "PuyoPuyoGame.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <QTimer>
#include <stack>
using namespace std;

PuyoPuyoGame::PuyoPuyoGame()
{
    ROW = 14;
    COL = 6;
    axis_row = 0;
    axis_col = COL / 2;
    score = 0;
    gameover = false;
    board.resize(ROW, vector<string>(COL, "Empty"));
    boardColors.resize(ROW, vector<QColor>(COL, QColor(255, 255, 255)));
    srand(time(0));

    shapes = {
        // Puyo shape
        {{0, 0, 0},
         {0, 1, 0},
         {0, 1, 0}}
    };

    shapeColors = {
        QColor(255, 0, 0),   // Red
        QColor(255, 255, 0), // Yellow
        QColor(0, 255, 0),   // Green
        QColor(0, 0, 255),   // Blue
        QColor(128, 0, 128)  // Purple
    };

    initializeBoard();
    generateNextShapes();
    spawnNewShape();
}

PuyoPuyoGame::~PuyoPuyoGame()
{
}

void PuyoPuyoGame::initializeBoard()
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

vector<vector<int>> PuyoPuyoGame::getRandomShape()
{
    return shapes[0];
}

vector<vector<QColor>> PuyoPuyoGame::getRandomColors()
{
    vector<vector<QColor>> colors(3, vector<QColor>(3, QColor(255, 255, 255)));
    colors[1][1] = shapeColors[rand() % shapeColors.size()];
    colors[2][1] = shapeColors[rand() % shapeColors.size()];
    return colors;
}

void PuyoPuyoGame::fixBlock()
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
}

QColor PuyoPuyoGame::darkenColor(const QColor& color)
{
    int factor = 50;
    return QColor(max(color.red() - factor, 0),
                  max(color.green() - factor, 0),
                  max(color.blue() - factor, 0));
}

QColor PuyoPuyoGame::brightenColor(const QColor& color)
{
    int factor = 140;
    return QColor(min(color.red() + factor, 255),
                  min(color.green() + factor, 255),
                  min(color.blue() + factor, 255));
}
void PuyoPuyoGame::clearConnectedTiles()
{
    bool tilesCleared = false;  // 타일이 제거되었는지 여부를 추적하는 변수
    vector<vector<bool>> visited(ROW, vector<bool>(COL, false));  // 방문 여부를 추적하는 2D 벡터
    vector<vector<int>> directions = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};  // 네 방향 (상, 하, 좌, 우)
    vector<pair<int, int>> connected;  // 연결된 타일의 좌표를 저장하는 벡터

    for (int i = 0; i < ROW; ++i)  // 보드의 각 행을 순회
    {
        for (int j = 0; j < COL; ++j)  // 보드의 각 열을 순회
        {
            if (board[i][j] == "FixedBlock" && !visited[i][j])  // 고정된 블록이면서 방문하지 않은 경우
            {
                connected.clear();  // 연결된 타일 초기화
                stack<pair<int, int>> stack;  // DFS를 위한 스택
                stack.push({i, j});  // 시작 타일을 스택에 추가
                QColor color = boardColors[i][j];  // 현재 타일의 색상

                while (!stack.empty())  // 스택이 비지 않은 동안 반복
                {
                    auto [x, y] = stack.top();  // 스택의 상단 요소 가져오기
                    stack.pop();  // 스택에서 제거
                    if (x < 0 || x >= ROW || y < 0 || y >= COL || visited[x][y] || board[x][y] != "FixedBlock" || boardColors[x][y] != color)
                    {
                        continue;  // 유효하지 않은 좌표이거나 다른 색상인 경우 무시
                    }

                    visited[x][y] = true;  // 현재 좌표를 방문으로 표시
                    connected.push_back({x, y});  // 연결된 타일에 추가

                    for (auto& dir : directions)  // 네 방향에 대해 반복
                    {
                        stack.push({x + dir[0], y + dir[1]});  // 다음 좌표를 스택에 추가
                    }
                }

                if (connected.size() >= 4)  // 연결된 타일이 4개 이상인 경우
                {
                    for (auto& pos : connected)  // 각 연결된 타일에 대해
                    {
                        boardColors[pos.first][pos.second] = brightenColor(boardColors[pos.first][pos.second]);  // 타일을 밝게 표시
                    }

                    // 타일을 밝게 표시한 후 제거하기 위한 지연
                    QTimer::singleShot(200, [this, connected, color]() {
                        for (auto& pos : connected)  // 각 연결된 타일에 대해
                        {
                            board[pos.first][pos.second] = "Empty";  // 타일 제거
                            boardColors[pos.first][pos.second] = QColor(255, 255, 255);  // 타일 색상 초기화
                        }

                        // 타일 제거 후 중력 적용
                        for (int col = 0; col < COL; ++col)  // 각 열에 대해
                        {
                            int empty_row = ROW - 1;  // 빈 행 초기화
                            for (int row = ROW - 1; row >= 0; --row)  // 아래에서 위로 순회
                            {
                                if (board[row][col] != "Empty")  // 타일이 있는 경우
                                {
                                    swap(board[row][col], board[empty_row][col]);  // 타일을 아래로 이동
                                    swap(boardColors[row][col], boardColors[empty_row][col]);  // 색상도 함께 이동
                                    --empty_row;  // 빈 행을 위로 이동
                                }
                            }
                        }

                        score += connected.size(); // 점수 업데이트
                    });
                }
            }
        }
    }
}

bool PuyoPuyoGame::checkGameOver()
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

vector<vector<QColor>> PuyoPuyoGame::getCurrentShapeColors()
{
    return currentShapeColors;
}

vector<vector<int>> PuyoPuyoGame::getCurrentShape()
{
    return currentShape;
}

void PuyoPuyoGame::rotateCurrentShapeClockwise()
{
    auto rotatedShape = rotateShape(currentShape, true);
    auto rotatedColors = rotateColors(currentShapeColors, true);
    if (canRotate(rotatedShape))
    {
        currentShape = rotatedShape;
        currentShapeColors = rotatedColors;
    }
}

void PuyoPuyoGame::rotateCurrentShapeCounterClockwise()
{
    auto rotatedShape = rotateShape(currentShape, false);
    auto rotatedColors = rotateColors(currentShapeColors, false);
    if (canRotate(rotatedShape))
    {
        currentShape = rotatedShape;
        currentShapeColors = rotatedColors;
    }
}

vector<vector<int>> PuyoPuyoGame::rotateShape(const vector<vector<int>>& shape, bool clockwise)
{
    int rows = shape.size();  // 블록 모양의 행 수
    int cols = shape[0].size();  // 블록 모양의 열 수
    vector<vector<int>> rotated(cols, vector<int>(rows, 0));  // 회전된 블록 모양을 저장할 벡터

    if (clockwise)  // 시계 방향으로 회전하는 경우
    {
        for (int i = 0; i < rows; ++i)  // 행을 순회
        {
            for (int j = 0; j < cols; ++j)  // 열을 순회
            {
                rotated[j][rows - 1 - i] = shape[i][j];  // 시계 방향 회전 로직
            }
        }
    }
    else  // 반시계 방향으로 회전하는 경우
    {
        for (int i = 0; i < rows; ++i)  // 행을 순회
        {
            for (int j = 0; j < cols; ++j)  // 열을 순회
            {
                rotated[cols - 1 - j][i] = shape[i][j];  // 반시계 방향 회전 로직
            }
        }
    }

    return rotated;  // 회전된 블록 모양 벡터 반환
}

vector<vector<QColor>> PuyoPuyoGame::rotateColors(const vector<vector<QColor>>& colors, bool clockwise)
{
    int rows = colors.size();  // 색상 행의 수
    int cols = colors[0].size();  // 색상 열의 수
    vector<vector<QColor>> rotated(cols, vector<QColor>(rows));  // 회전된 색상을 저장할 벡터

    if (clockwise)  // 시계 방향으로 회전하는 경우
    {
        for (int i = 0; i < rows; ++i)  // 행을 순회
        {
            for (int j = 0; j < cols; ++j)  // 열을 순회
            {
                rotated[j][rows - 1 - i] = colors[i][j];  // 시계 방향 회전 로직
            }
        }
    }
    else  // 반시계 방향으로 회전하는 경우
    {
        for (int i = 0; i < rows; ++i)  // 행을 순회
        {
            for (int j = 0; j < cols; ++j)  // 열을 순회
            {
                rotated[cols - 1 - j][i] = colors[i][j];  // 반시계 방향 회전 로직
            }
        }
    }

    return rotated;  // 회전된 색상 벡터 반환
}


bool PuyoPuyoGame::canRotate(const vector<vector<int>>& rotatedShape)
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

bool PuyoPuyoGame::canMoveDown()
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

void PuyoPuyoGame::dropBlock()
{
    while (canMoveDown())
    {
        axis_row++;
    }
    fixBlock();
    clearConnectedTiles();
    spawnNewShape();
}

void PuyoPuyoGame::generateNextShapes()
{
    while (nextQueue.size() < 2)
    {
        nextQueue.push({getRandomShape(), getRandomColors()});
    }
}

void PuyoPuyoGame::spawnNewShape()
{
    axis_row = 0;
    axis_col = COL / 2;
    auto nextShape = nextQueue.front();
    currentShape = nextShape.first;
    currentShapeColors = nextShape.second;
    nextQueue.pop();
    generateNextShapes();
}
