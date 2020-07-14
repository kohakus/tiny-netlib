#ifndef NETLIBCC_CORE_MUTEX_H_
#define NETLIBCC_CORE_MUTEX_H_

#include <pthread.h>
#include <cassert>

#include "netlibcc/core/ThisThread.h"
#include "netlibcc/core/Noncopyable.h"

namespace netlibcc {

// a simple wrapper of POSIX mutex
class Mutex : Noncopyable {
public:
    Mutex() : holder_(0) {
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~Mutex() {
        // make sure mutex is not hold by any thread
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread() const {
        return holder_ == thisthread::tid();
    }

    void lockedAssert() const {
        assert(isLockedByThisThread());
    }

    // provide basic interface for higher level components
    void lock() {
        // the order is important
        pthread_mutex_lock(&mutex_);
        setHolder();
    }

    void unlock() {
        // the order is important
        unsetHolder();
        pthread_mutex_unlock(&mutex_);
    }

    // mainly used by condition variable
    pthread_mutex_t* getPthreadMutex() {
        return &mutex_;
    }

private:
    // Condition will use HolderGuard
    friend class Condition;

    void setHolder() {
        holder_ = thisthread::tid();
    }

    void unsetHolder() {
        holder_ = 0;
    }

    // an interface used by Condition for modifying holder information (RAII)
    class HolderGuard : Noncopyable {
    public:
        explicit HolderGuard(Mutex& owner) : owner_(owner) {
            owner_.unsetHolder();
        }

        ~HolderGuard() {
            owner_.setHolder();
        }

    private:
        Mutex& owner_;
    };

    // Mutex data
    pthread_mutex_t mutex_;
    pid_t holder_;
};


// mutex lock_guard (RAII)
class MutexLockGuard : Noncopyable {
public:
    explicit MutexLockGuard(Mutex& mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    Mutex& mutex_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_MUTEX_H_