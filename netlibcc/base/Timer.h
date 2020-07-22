#ifndef NETLIBCC_BASE_TIMER_H_
#define NETLIBCC_BASE_TIMER_H_

#include <atomic>
#include <functional>
#include "netlibcc/core/TimeAnchor.h"
#include "netlibcc/core/Noncopyable.h"

namespace netlibcc {
namespace net {

// class for timer task, driven by timer event
class Timer : Noncopyable {
public:
    Timer(std::function<void()> callback, TimeAnchor expiration, double interval);

    void run() const {
        callback_();
    }

    TimeAnchor expiration() const {
        return expiration_;
    }

    int64_t seqId() const {
        return seqid_;
    }

    int timerPos() const {
        return timer_pos_;
    }

    bool inHeap() const {
        return timer_pos_ >= 0;
    }

    static int64_t numCreated() {
        return numCreated_.load();
    }

    void setPosInHeap(size_t heap_pos);
    void setPosInHeap(const int heap_pos);

private:
    // timer callback function
    const std::function<void()> callback_;

    //* timer timming
    TimeAnchor expiration_;
    const double interval_;

    // the total number of created timer
    static std::atomic_int64_t numCreated_;

    // the unique number id in created timer sequence
    const int64_t seqid_;

    // the position where timer obj in timer heap (see TimerContainer)
    int timer_pos_;
};

} // namespace net
} // namespace netlibcc


#endif // NETLIBCC_BASE_TIMER_H_