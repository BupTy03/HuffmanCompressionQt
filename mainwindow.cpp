#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "huffmanencoding.hpp"
#include "packagedtask.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , secondTread_{new QThread}
    , compressingTask_{new PackagedTask}
{
    ui->setupUi(this);
    setAcceptDrops(true);

    QObject::connect(ui->exitPushButton, &QPushButton::clicked, qApp, &QApplication::exit);
    QObject::connect(ui->fromViewPushButton, &QPushButton::clicked, [this] {
        ui->fromLineEdit->setText(QFileDialog::getOpenFileName(this));
    });

    QObject::connect(ui->toViewPushButton, &QPushButton::clicked, [this] {
        ui->toLineEdit->setText(QFileDialog::getSaveFileName(this));
    });

    QObject::connect(ui->startPushButton, &QPushButton::clicked, this, &MainWindow::startProcess);

    QObject::connect(this, &MainWindow::startCompressingTask, compressingTask_, &PackagedTask::startTask);
    QObject::connect(compressingTask_, &PackagedTask::taskDone, this, &MainWindow::endProcessing);
    compressingTask_->moveToThread(secondTread_.get());
    secondTread_->start();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::dropEvent(QDropEvent* ev)
{
    assert(ev != nullptr);
    if(processStarted_) {
        return;
    }

    const auto urls = (ev->mimeData())->urls();
    if(urls.isEmpty()) {
       return;
    }

    const auto& path = urls.first();
    ui->fromLineEdit->setText(path.url(QUrl::UrlFormattingOption::PreferLocalFile));

    ev->acceptProposedAction();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* ev)
{
    assert(ev != nullptr);
    if(processStarted_ || !(ev->mimeData())->hasUrls()) {
       return;
    }
    ev->acceptProposedAction();
}

void MainWindow::startProcess()
{
    const auto pathFromFile = ui->fromLineEdit->text();
    if(pathFromFile.isEmpty()) {
        showError("Path to source file is empty!");
        return;
    }

    if(!QFileInfo::exists(pathFromFile)) {
        showError("Path to source file is not exists!");
        return;
    }

    const auto pathToFile = ui->toLineEdit->text();
    if(pathToFile.isEmpty()) {
        showError("Path to output file is empty!");
        return;
    }

    const auto dir = QFileInfo(pathToFile).dir();
    if(!dir.exists()) {
        showError("Path to output file is not exists!");
        return;
    }

    const auto pathFrom = pathFromFile.toStdString();
    const auto pathTo = pathToFile.toStdString();
    if(ui->compressRadioButton->isChecked()) {
        compressingTask_->setTask([pathFrom, pathTo]{
            try {
                compress_file(pathFrom, pathTo);
            }
            catch(const std::exception& exc) {
                std::cerr << "Exception while compressing file: " << exc.what();
                return false;
            }
            return true;
        });
    }
    else {
        compressingTask_->setTask([pathFrom, pathTo]{
            try {
                decompress_file(pathFrom, pathTo);
            }
            catch(const std::exception& exc) {
                std::cerr << "Exception while decompressing file: " << exc.what();
                return false;
            }
            return true;
        });
    }

    beginProcessing();
    emit startCompressingTask();
}

void MainWindow::beginProcessing()
{
    processStarted_ = true;
    blockControls(true);
    setStatusTip("Processing file...");
}

void MainWindow::endProcessing(bool success)
{
    if(success) {
        setStatusTip("Done");
    }
    else {
        showError("Unable to compress/decompress file");
        setStatusTip("Error");
    }

    processStarted_ = false;
    blockControls(false);
}

void MainWindow::showError(const QString& message) { QMessageBox::critical(this, QObject::tr("Error"), message); }

void MainWindow::blockControls(bool flag)
{
    ui->fromLineEdit->setDisabled(flag);
    ui->toLineEdit->setDisabled(flag);

    ui->fromViewPushButton->setDisabled(flag);
    ui->toViewPushButton->setDisabled(flag);

    ui->compressRadioButton->setDisabled(flag);
    ui->decompressRadioButton->setDisabled(flag);

    ui->exitPushButton->setDisabled(flag);
    ui->startPushButton->setDisabled(flag);
}
