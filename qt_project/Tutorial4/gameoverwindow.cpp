#include "GameoverWindow.h"

GameoverWindow::GameoverWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Game over");
    resize(384, 216);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("Game over", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QPushButton *okButton = new QPushButton("OK", this);
    layout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, this, &GameoverWindow::onOkButtonClicked);

    setLayout(layout);
}

GameoverWindow::~GameoverWindow()
{
    ;
}

void GameoverWindow::onOkButtonClicked()
{
    QApplication::quit();
}
