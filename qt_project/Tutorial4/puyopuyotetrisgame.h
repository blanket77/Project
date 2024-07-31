#ifndef PUYO_PUYO_TETRIS_GAME_H
#define PUYO_PUYO_TETRIS_GAME_H

#include <vector>
#include <QColor>
#include <QObject>
#include <queue>
using namespace std;

class PuyoPuyoTetrisGame : public QObject
{
    Q_OBJECT

public:
    PuyoPuyoTetrisGame();
    ~PuyoPuyoTetrisGame();

    int ROW;
    int COL;
    int axis_row;
    int axis_col;
    int score;
    bool gameover;
    bool isPuyo;

    vector<vector<string>> board;
    vector<vector<QColor>> boardColors;
    vector<vector<int>> currentShape;
    vector<vector<QColor>> currentShapeColors;
    queue<pair<vector<vector<int>>, vector<vector<QColor>>>> nextQueue;

    vector<vector<vector<int>>> puyoShapes;
    vector<vector<vector<int>>> tetrisShapes;
    vector<QColor> puyoColors;
    vector<QColor> tetrisColors;

    void initializeBoard();
    vector<vector<int>> getRandomShape();
    vector<vector<QColor>> getRandomColors();
    void fixBlock();
    QColor darkenColor(const QColor& color);
    QColor brightenColor(const QColor& color);
    void clearConnectedTiles();
    bool checkGameOver();
    vector<vector<int>> getCurrentShape();
    vector<vector<QColor>> getCurrentShapeColors();
    void rotateCurrentShapeClockwise();
    void rotateCurrentShapeCounterClockwise();
    vector<vector<int>> rotateShape(const vector<vector<int>>& shape, bool clockwise);
    vector<vector<QColor>> rotateColors(const vector<vector<QColor>>& colors, bool clockwise);
    bool canRotate(const vector<vector<int>>& rotatedShape);
    bool canMoveDown();
    void dropBlock();
    void generateNextShapes();
    void spawnNewShape();
    void applyGravity();

signals:
    void tilesCleared();
    void gameOver();
};

#endif // PUYO_PUYO_TETRIS_GAME_H
