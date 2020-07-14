#ifndef NETLIBCC_CORE_THREAD_H_
#define NETLIBCC_CORE_THREAD_H_

#include <pthread.h>
#include <memory>
#include <atomic>
#include <string>
#include <functional>

#include "netlibcc/core/CountDownLatch.h"

namespace netlibcc {

// all the created threads should be managed by this class
class Thread : Noncopyable {
public:
    // use function object to represent thread function,
    // which can benefit from functional methods
    using ThreadFunc = std::function<void ()>;

    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();

    // thread control methods
    void start();
    int join();

    // thread state acquire
    bool started() const { return started_; }

    pid_t tid() const { return tid_; }

    const std::string& name() const { return name_; }

    // get created threads number
    static int getNumCreated() { return numCreated_.load(); }

private:
    // use this method if the thread name is not specified
    void setDefaultName();

    // increment and get created num
    int incNum() { return ++numCreated_; }

    // data members
    bool           started_;
    bool           joined_;
    pthread_t      pthreadId_;
    pid_t          tid_;
    ThreadFunc     func_;
    std::string    name_;
    CountDownLatch latch_;

    // atomic data to restore the number of created threads
    static std::atomic_int32_t numCreated_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_THREAD_H_