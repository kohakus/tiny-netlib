#ifndef NETLIBCC_CORE_CONDITION_H_
#define NETLIBCC_CORE_CONDITION_H_

#include "netlibcc/core/Mutex.h"
#include <pthread.h>

namespace netlibcc {

// a simple wrapper of POSIX condition variable
class Condition : Noncopyable {
public:
    explicit Condition(Mutex& mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, nullptr);
    }

    ~Condition() {
        pthread_cond_destroy(&cond_);
    }

    void wait() {
        Mutex::HolderGuard hg(mutex_);
        pthread_cond_wait(&cond_, mutex_.getPthreadMutex());
    }

    bool waitSecs(double seconds);

    void notify() {
        pthread_cond_signal(&cond_);
    }

    // wake all threads that sleep on condition variable
    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }

private:
    Mutex& mutex_;
    pthread_cond_t cond_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_CONDITION_H_