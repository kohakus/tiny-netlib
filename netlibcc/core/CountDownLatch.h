#ifndef NETLIBCC_CORE_COUNTDOWNLATCH_H_
#define NETLIBCC_CORE_COUNTDOWNLATCH_H_

#include "netlibcc/core/Condition.h"
#include "netlibcc/core/Mutex.h"

namespace netlibcc {

// CountDownLatch is useful for waiting thread initialization
class CountDownLatch : Noncopyable {
public:
    // ctor
    explicit CountDownLatch(int count);

    // wait for count decrease to zero
    void wait();

    // decrease count
    void countDown();

    int getCount() const;

private:
    // the order is important
    mutable Mutex mutex_;
    Condition condition_;
    int count_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_COUNTDOWNLATCH_H_