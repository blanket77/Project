#ifndef GAMEOVERWINDOW_H
#define GAMEOVERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QApplication>

class GameoverWindow : public QWidget
{
    Q_OBJECT

public:
    GameoverWindow(QWidget *parent = nullptr);
    ~GameoverWindow();

private slots:
    void onOkButtonClicked();
};

#endif // GAMEOVERWINDOW_H
