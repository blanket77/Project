#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QLabel"
#include "TetrisWindow.h"
#include "PuyoPuyoWindow.h"
#include "TetrisWindow.h"
#include "PuyopuyotetrisWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this, &MainWindow::buttonClicked, this, &MainWindow::generateGame);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonPuyopuyo_clicked()
{
    emit buttonClicked(QString("Puyopuyo"));
}

void MainWindow::on_pushButtonTetris_clicked()
{
    emit buttonClicked(QString("Tetris"));
}

void MainWindow::on_pushButtonPuyopuyoTetris_clicked()
{
    emit buttonClicked(QString("PuyopuyoTetris"));
}

void MainWindow::generateGame(QString gamename)
{   if(gamename == "Tetris"){
        TetrisWindow* tetriswindow = new TetrisWindow();
        tetriswindow->show();
    }
    else if(gamename == "Puyopuyo"){
        PuyoPuyoWindow* puyopuyowindow = new PuyoPuyoWindow();
        puyopuyowindow->show();
    }
    else if(gamename == "PuyopuyoTetris"){
        PuyoPuyoTetrisWindow* ptyopuyotetrisWindow = new PuyoPuyoTetrisWindow();
        ptyopuyotetrisWindow->show();
    }
    else {}

    //GameoverWindow* gameoverwindow = new GameoverWindow;
    //gameoverwindow->show();
    this->close();
}
