#include "netlibcc/net/IOThread.h"
#include "netlibcc/net/EventLoop.h"

namespace netlibcc {
namespace net {

IOThread::IOThread(const ThreadInitCallback& cb, const std::string& name)
    : exiting_(false),
      mutex_(),
      loop_(nullptr),
      thread_(std::bind(&IOThread::threadFunc, this), name),
      cond_(mutex_),
      init_callback_(cb) {}

IOThread::~IOThread() {
    // set flag
    exiting_ = true;

    if (loop_) {
        loop_->quit();
        thread_.join();
    }
}

void IOThread::threadFunc() {
    EventLoop loop;
    // the callback function called during initialization
    if (init_callback_) {
        init_callback_(&loop);
    }

    // (loop_ can also be accessed by the thread which creates the IOThread obj)
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();

    MutexLockGuard lock(mutex_);
    loop_ = nullptr;
}

EventLoop* IOThread::start() {
    thread_.start();
    EventLoop* loop = nullptr;

    // wait until loop_ is set
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait();
        }
        loop = loop_;
    }

    return loop;
}

} // namespace net
} // namespace netlibcc