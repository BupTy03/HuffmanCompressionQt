#ifndef PACKAGEDTASK_HPP
#define PACKAGEDTASK_HPP

#include <QObject>

#include <functional>
#include <exception>


class PackagedTask : public QObject
{
    Q_OBJECT
public:
    using Task = std::function<void()>;

public:
    void setTask(Task task) { task_ = std::move(task); }
    std::exception_ptr getLastException() const { return pException_; }

signals:
    void taskDone(bool success);

public slots:
    void startTask()
    {
        try {
            task_();
            emit taskDone(true);
        }
        catch(...) {
            pException_ = std::current_exception();
            emit taskDone(false);
        }
    }

private:
    Task task_;
    std::exception_ptr pException_;
};

#endif // PACKAGEDTASK_HPP
