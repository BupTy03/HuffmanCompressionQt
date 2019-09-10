#ifndef PACKAGEDTASK_HPP
#define PACKAGEDTASK_HPP

#include <QObject>

#include <functional>

class PackagedTask : public QObject
{
    Q_OBJECT
public:
    void setTask(std::function<bool()> task) { tsk_ = std::move(task); }

signals:
    void taskDone(bool success);

public slots:
    void startTask() { const bool successfully = tsk_(); emit taskDone(successfully); }

private:
    std::function<bool()> tsk_;
};

#endif // PACKAGEDTASK_HPP
