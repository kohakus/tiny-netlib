#include "netlibcc/core/CountDownLatch.h"

netlibcc::CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count) {}

void netlibcc::CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);

    // loop for spurious wakeup
    while (count_ > 0) {
        condition_.wait();
    }
}

void netlibcc::CountDownLatch::countDown() {
    MutexLockGuard lock(mutex_);
    --count_;

    // use notify all to wakeup all related threads
    if (count_ == 0) {
        condition_.notifyAll();
    }
}

int netlibcc::CountDownLatch::getCount() const {
    MutexLockGuard lock(mutex_);
    return count_;
}