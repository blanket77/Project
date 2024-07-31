#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <vector>
#include <queue>
#include <string>
#include <map>
#include <QColor>
#include <QObject>
using namespace std;

class TetrisGame : public QObject
{
    Q_OBJECT
public:
    int ROW; //게임 보드의 행 수
    int COL; // 게임 보드의 열 수
    double axis_row; // 현재 블록의 중심의 행 위치
    int score; //현재 게임 점수
    double axis_col; //현재 블록의 중심의 열 위치
    bool gameover; //게임 상태 (게임 오버 시 true)
    vector<vector<string>> board; //문자열로 표현된 게임 보드
    vector<vector<int>> currentShape; // 현재 게임의 블록
    vector<vector<QColor>> boardColors; // 게임 보드 셀의 색상
    queue<int> nextQueue; //다음에 나올 블록의 큐
    int currentShapeIndex; // 현재 블록의 인덱스
    map<int, QColor> shapeColors; //블록 인덱스와 그 색상의 매핑
    vector<vector<vector<int>>> shapes; //블록의 모양들
    int rotationState;//블록의 현재 회전 상태

    TetrisGame();
    ~TetrisGame();

    void initializeBoard(); // 게임 보드를 초기화
    int getRandomShape(); //랜덤한 블록 인덱스를 생성
    void dropBlock(); // 현재 블록을 한 줄 아래로 내림
    bool checkGameOver(); //게임 오버인지 확인
    vector<vector<int>> getCurrentShape(); //현재 블록을 반환
    void fixBlock(); //현재 블록을 보드에 고정
    void rotateCurrentShapeClockwise(); //현재 블록을 시계 방향으로 회전
    void generateNextShapes(); // 다음 블록들을 생성
    void clearFullLines(); //가득 찬 줄을 클리어
    void rotateCurrentShapeCounterClockwise(); //현재 블록을 반시계 방향으로 회전
    void spawnNewShape(); //새로운 블록을 가져온다.

signals:
    void linesCleared(); // 줄이 클리어 되었음을 알리는 신호
    void gameOver(); // 게임 오버를 알리는 신호

private:
    vector<vector<int>> rotateShape(const vector<vector<int>>& shape, bool clockwise); //블록을 회전시키는 함수
    bool canRotate(const vector<vector<int>>& rotatedShape); // 블록 회전 가능?
    bool canMoveDown();//블록을 아래로 가능?
    QColor darkenColor(const QColor& color); //색을 어둡게
    QColor brightenColor(const QColor& color); //색을 밝게
};

#endif // TETRISGAME_H
