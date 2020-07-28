#include "netlibcc/net/IOThreadPool.h"

#include <fmt/format.h>
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/net/IOThread.h"

namespace netlibcc {
namespace net {

IOThreadPool::IOThreadPool(EventLoop* base_loop, StringArg name)
        : base_loop_(base_loop),
          name_(name.c_str()),
          num_threads_(0),
          next_(0),
          started_(false) {}

IOThreadPool::~IOThreadPool() {}

// usually used before threadpool start
void IOThreadPool::setThreadNum(int num) {
    num_threads_ = num;
}

void IOThreadPool::start(const ThreadInitCallback& cb) {
    base_loop_->assertInLoopThread();
    assert(!started_);

    // set flag
    started_ = true;

    // create and start N threads (N is set by setThreadNum)
    for (int i = 0; i < num_threads_; i++) {
        std::unique_ptr<IOThread> new_thread(new IOThread(cb, fmt::format("{}{}", name_, i)));
        loops_.push_back(new_thread->start());
        threads_.push_back(std::move(new_thread));
    }
    // the case that only the current IOThread is available
    if (num_threads_ == 0 && cb) { cb(base_loop_); }
}

EventLoop* IOThreadPool::getNextLoop() {
    base_loop_->assertInLoopThread();
    assert(started_);

    // if no other thread in pool, just use base loop thread
    EventLoop* loop = base_loop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_++];
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

} // namespace net
} // namespace netlibcc