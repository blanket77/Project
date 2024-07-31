#ifndef PUYO_PUYO_GAME_H
#define PUYO_PUYO_GAME_H

#include <vector>
#include <QColor>
#include <QObject>
#include <queue>
using namespace std;

class PuyoPuyoGame : public QObject
{
    Q_OBJECT

public:
    PuyoPuyoGame();
    ~PuyoPuyoGame();

    int ROW;
    int COL;
    int axis_row;
    int axis_col;
    int score;
    bool gameover;

    vector<vector<string>> board;
    vector<vector<QColor>> boardColors;
    vector<vector<int>> currentShape;
    vector<vector<QColor>> currentShapeColors;
    queue<pair<vector<vector<int>>, vector<vector<QColor>>>> nextQueue;

    vector<vector<vector<int>>> shapes;
    vector<QColor> shapeColors;
    vector<vector<QColor>> rotateColors(const vector<vector<QColor>>& colors, bool clockwise);

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
    bool canRotate(const vector<vector<int>>& rotatedShape);
    bool canMoveDown();
    void dropBlock();
    void generateNextShapes();
    void spawnNewShape();

signals:
    void tilesCleared();
    void gameOver();
};

#endif // PUYO_PUYO_GAME_H
