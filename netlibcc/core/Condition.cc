#include "netlibcc/core/Condition.h"
#include <errno.h>
#include <cstdint>

// a method similar to timedwait
// the return value is true if time out
bool netlibcc::Condition::waitSecs(double seconds) {
    // note that pthread_cond_timedwait need absolute time
    struct timespec abstime;

    // get wall-time by clock_gettime, which provides nanoseconds
    clock_gettime(CLOCK_REALTIME, &abstime);

    const int64_t nanoweight = 1000000000;
    int64_t nanosecs = static_cast<int64_t>(seconds * nanoweight);

    // deal with nsec part correctly
    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanosecs) / nanoweight);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanosecs) % nanoweight);

    Mutex::HolderGuard hg(mutex_);
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getPthreadMutex(), &abstime);
}