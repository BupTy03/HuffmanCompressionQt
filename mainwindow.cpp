#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "huffmanencoding.hpp"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->exitPushButton, &QPushButton::clicked, qApp, &QApplication::exit);
    QObject::connect(ui->viewPushButton, &QPushButton::clicked, [this] {
        ui->lineEdit->setText(QFileDialog::getOpenFileName(this, "", "/"));
    });

    QObject::connect(ui->startPushButton, &QPushButton::clicked, [this]{
        const QString pathToFile = ui->lineEdit->text();
        if(pathToFile.isEmpty()) {
            return;
        }
        const std::string pathFrom = pathToFile.toStdString();

        const QString path = QFileDialog::getSaveFileName(this, "", "/");
        if(path.isEmpty()) {
            return;
        }

        const std::string pathTo = path.toStdString();

        if(ui->compressRadioButton->isChecked()) {
            compress_file(pathFrom, pathTo);
        }
        else {
            decompress_file(pathFrom, pathTo);
        }
    });
}

MainWindow::~MainWindow() { delete ui; }
