#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonPuyopuyo_clicked();
    void on_pushButtonTetris_clicked();
    void on_pushButtonPuyopuyoTetris_clicked();

    void generateGame(QString gamename);

signals:
    void buttonClicked(QString gamename);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
