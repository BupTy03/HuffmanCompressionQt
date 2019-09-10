#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QDoubleValidator>
#include <QThread>

#include <memory>

namespace Ui {
class MainWindow;
}

struct QThreadDeleter {
    void operator()(QThread* pThread) const {
        pThread->quit();
        pThread->wait();
        delete pThread;
    }
};

using QThreadPtr = std::unique_ptr<QThread, QThreadDeleter>;

class PackagedTask;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void dropEvent(QDropEvent* ev) override;
    void dragEnterEvent(QDragEnterEvent* ev) override;

signals:
    void startCompressingTask();

private slots:
    void startProcess();
    void beginProcessing();
    void endProcessing(bool success);

private:
    void showError(const QString& message);
    void blockControls(bool flag);

private:
    Ui::MainWindow* ui = nullptr;
    bool processStarted_ = false;
    QThreadPtr secondTread_;
    PackagedTask* compressingTask_ = nullptr;
};

#endif // MAINWINDOW_HPP
