#ifndef NETLIBCC_CORE_THREADPOOL_H_
#define NETLIBCC_CORE_THREADPOOL_H_

#include <vector>
#include <deque>

#include "netlibcc/core/Condition.h"
#include "netlibcc/core/Mutex.h"
#include "netlibcc/core/Thread.h"

namespace netlibcc {

class ThreadPool : Noncopyable {
public:
    using Task = std::function<void ()>;

    // ctor and dtor
    explicit ThreadPool(const std::string& name = std::string("thread pool"));
    ~ThreadPool();

    // settings before threads start
    void setMaxQueSize(int max_size) { max_quesize_ = max_size; }
    void setInitCallback(const Task& cb) { init_callback_ = cb; }

    // thread pool control
    void start(int num_thread);
    void stop();

    // add new task to task queue
    void addTask(Task);

    // member aquire
    const std::string& name() const { return name_; }
    size_t numTasks() const;

private:
    bool isFull() const;
    void runInThread();
    Task take();

    // after thread pool start, running_ is true
    bool running_;
    std::string name_;
    Task init_callback_;
    std::vector<std::unique_ptr<Thread>> threads_;
    std::deque<Task> task_queue_;
    size_t max_quesize_;

    // data members for synchronization
    mutable Mutex mutex_;
    Condition not_empty_;
    Condition not_full_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_THREADPOOL_H_