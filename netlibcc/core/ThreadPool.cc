#include "netlibcc/core/ThreadPool.h"

#include <cstdio>
#include <cassert>

namespace netlibcc {

ThreadPool::ThreadPool(const std::string& name)
            : running_(false),
              name_(name),
              max_quesize_(0),
              mutex_(),
              not_empty_(mutex_),
              not_full_(mutex_) {}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

size_t ThreadPool::numTasks() const {
    MutexLockGuard lock(mutex_);
    return task_queue_.size();
}

// used with not_full_ condition
bool ThreadPool::isFull() const {
    // condition need a locked mutex
    mutex_.lockedAssert();
    return max_quesize_ > 0 && task_queue_.size() >= max_quesize_;
}

void ThreadPool::start(int num_thread) {
    // all the threads are started in a clean environment
    // currently we only have main thread
    if (num_thread == 0) { return; }
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(num_thread);

    // start threads
    for (int i = 0; i < num_thread; i++) {
        char buf[32];
        // thread id: 1, 2, 3, ...
        snprintf(buf, sizeof buf, "%d", i+1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), name_+buf));
        threads_[i]->start();
    }

    // Note that in this implementation, main thread do not run tasks in task queue.
    // If the num_thread is given zero, the thread pool will not start.
}

// thread running function
void ThreadPool::runInThread() {
    if (init_callback_) { init_callback_(); }
    // repeatedly take task and run
    while (running_) {
        Task task(take());
        if (task) { task(); }
    }
}

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    // wait for available task
    while (task_queue_.empty() && running_) {
        not_empty_.wait();
    }

    Task task;
    if (!task_queue_.empty()) {
        // now we can take a task from task queue
        task = task_queue_.front();
        task_queue_.pop_front();
        if (max_quesize_ > 0) {
            not_full_.notify();
        }
    }
    return task;
}

void ThreadPool::addTask(Task task) {
    MutexLockGuard lock(mutex_);
    // if the task queue is full, we should wait
    while (isFull() && running_) {
        not_full_.wait();
    }
    // check again
    if (!running_) { return; }
    assert(!isFull());

    // add a task to task queue
    task_queue_.push_back(std::move(task));
    not_empty_.notify();
}

void ThreadPool::stop() {
    // reset current ThreadPool state
    {
        MutexLockGuard lock(mutex_);
        running_ = false;
        not_empty_.notifyAll();
        not_full_.notifyAll();
    }

    for (auto& thread : threads_) {
        thread->join();
    }
}

} // namespace netlibcc