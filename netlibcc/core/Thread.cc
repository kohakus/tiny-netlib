#include "netlibcc/core/Thread.h"

#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/prctl.h>

#include <cstdio>
#include <type_traits>

#include "netlibcc/core/ThisThread.h"

namespace netlibcc {

// implementation of thread related components
namespace thread_impl {

// thread data, passing to pthread starting function
struct ThreadData {
    using ThreadFunc = netlibcc::Thread::ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(ThreadFunc func,
               const std::string& name,
               pid_t* tid,
               CountDownLatch* latch)
        : func_(std::move(func)),
          name_(name),
          tid_(tid),
          latch_(latch) {}

    ThreadData(ThreadFunc&& func,
               const std::string& name,
               pid_t* tid,
               CountDownLatch* latch)
        : func_(func),
          name_(name),
          tid_(tid),
          latch_(latch) {}

    // run what we want -> func_
    void runInThread() {
        // thread data initialization
        // thread identification setting (thread local data)
        *tid_ = netlibcc::thisthread::tid();
        tid_ = nullptr;

        // CounDownLatch decrease
        latch_->countDown();
        latch_ = nullptr;

        // set TLS thread name data
        netlibcc::thisthread::t_threadName = name_.c_str();
        ::prctl(PR_SET_NAME, netlibcc::thisthread::t_threadName);

        // run thread func
        func_();
        netlibcc::thisthread::t_threadName = "finished";
    }
};

// thread identification generation
pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

// pthread starting function
void* startThread(void* tdata) {
    ThreadData* data = static_cast<ThreadData*>(tdata);
    data->runInThread();
    delete data;
    return nullptr;
}

} // namespace thread_impl

void thisthread::cacheTid() {
    assert(t_cachedTid == 0);
    t_cachedTid = thread_impl::gettid();
    t_tidStrLen = snprintf(t_tidStr, sizeof t_tidStr, "%5d ", t_cachedTid);
}

// static atomic direct-initialisation
std::atomic_int32_t Thread::numCreated_(0);

// Thread ctor
Thread::Thread(ThreadFunc func, const std::string& name)
            : started_(false),
              joined_(false),
              pthreadId_(0),
              tid_(0),
              func_(std::move(func)),
              name_(name),
              latch_(1) {
    setDefaultName();
}

// make thread detached, let system to recycle resources
// (if it is not joined, but we do not need it any more, or it is going to be destroyed)
Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName() {
    int num = incNum();
    // the thread name is not specified
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    thread_impl::ThreadData* data = new thread_impl::ThreadData(func_, name_, &tid_, &latch_);
    // get zero if a thread is created successfully
    if (!pthread_create(&pthreadId_, NULL, &thread_impl::startThread, data)) {
        latch_.wait();
        assert(tid_ > 0);
    } else {
        // bad pthread create
        started_ = false;
        delete data;
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}

} // namespace netlibcc