#include "netlibcc/net/TimerContainer.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <cassert>

#include "netlibcc/net/TimerId.h"
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/base/Timer.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {
namespace net {
namespace detail {

// ********** (From Linux Manual Page) *********
//    struct timespec {
//        time_t tv_sec;    /* Seconds */
//        long   tv_nsec;   /* Nanoseconds */
//    };
//
//    struct itimerspec {
//        struct timespec it_interval;  /* Interval for periodic timer */
//        struct timespec it_value;     /* Initial expiration */
//    };
// ********** (From Linux Manual Page) *********

int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create()";
    }
    return timerfd;
}

struct timespec timeIntervalFromNow(TimeAnchor when) {
    int64_t microseconds = when.timeMicro() - TimeAnchor::now().timeMicro();
    if (microseconds < 100) {
        microseconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / TimeAnchor::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(1000 * (microseconds % TimeAnchor::kMicroSecondsPerSecond));
    return ts;
}

void resetTimerfd(int timerfd, TimeAnchor expiration) {
    struct itimerspec new_value;
    struct itimerspec old_value;
    memset(&new_value, 0, sizeof new_value);
    memset(&old_value, 0, sizeof old_value);

    new_value.it_value = timeIntervalFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret) {
        LOG_SYSERR << "timerfd_settime()";
    }
}

void readTimerfd(int timerfd, TimeAnchor now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerContainer::handleRead() " << howmany << " at " << now.toStr();
    if (n != sizeof howmany) {
        LOG_ERROR << "TimerContainer::handleRead() reads " << n << " bytes instead of 8";
    }
}

inline size_t heapNodeLeftChild(size_t pos) {
    return pos + pos + 1;
}

inline size_t heapNodeRightChild(size_t pos) {
    return pos + pos + 2;
}

inline size_t heapNodeParent(size_t pos) {
    return pos == 0 ? 0 : ((pos - 1) / 2);
}

inline bool cmpTimer(const Timer* lhs, const Timer* rhs) {
    return (lhs->expiration()).timeMicro() < (rhs->expiration()).timeMicro();
}

const int kInvalidHeapPosition = -1;
const int kTopHeapPosition = 0;

} // namespace detail

// ctor
TimerContainer::TimerContainer(EventLoop* loop)
        : timer_list_(),
          heap_size_(0),
          loop_(loop),
          timerfd_(detail::createTimerfd()),
          timerfd_channel_(loop, timerfd_),
          calling_expired_timers_(false) {
    // the timerfd is readable if timeout event occur
    timerfd_channel_.setReadCallback(std::bind(&TimerContainer::handleRead, this));
    timerfd_channel_.enableReading();
}

// dtor
TimerContainer::~TimerContainer() {
    timerfd_channel_.disableAll();
    timerfd_channel_.remove();
    ::close(timerfd_);
    for (Timer* timer : timer_list_) {
        delete timer;
    }
}

//* interface of TimerContainer

TimerId TimerContainer::addTimer(TimerCallback cb,
                                 TimeAnchor expiration,
                                 double interval) {
    Timer* timer = new Timer(std::move(cb), expiration, interval);
    loop_->runInLoop(std::bind(&TimerContainer::addTimerInLoop, this, timer));
    return TimerId(timer, timer->seqId());
}

void TimerContainer::cancel(TimerId timerid) {
    loop_->runInLoop(std::bind(&TimerContainer::cancelInLoop, this, timerid));
}

//* functions to support work-thread-transfer strategy

void TimerContainer::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    bool heap_empty = isHeapEmpty();
    TimeAnchor old_earliest;
    if (!heap_empty) {
        old_earliest = heapTop()->expiration();
    }

    // add new timer to timer heap
    heapAdd(timer);

    // reset timerfd if earliest expiration time changed
    if (heap_size_ == 1 || heapTop()->expiration() < old_earliest) {
        detail::resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerContainer::cancelInLoop(TimerId timerid) {
    loop_->assertInLoopThread();
    Timer* cancel_timer = timerid.timer_;

    // check if cancel_timer is in TimerContainer
    if (cancel_timer->inHeap() &&
            static_cast<size_t>(cancel_timer->timerPos()) < heap_size_) {
        heapDel(cancel_timer);
        delete cancel_timer;
    }
    assert(heap_size_ == timer_list_.size());
}

//* functions to deal with timerfd alarm

void TimerContainer::handleRead() {
    loop_->assertInLoopThread();
    TimeAnchor now(TimeAnchor::now());

    // read timerfd
    detail::readTimerfd(timerfd_, now);

    // get all expired timers
    TimerList expired = getExpiredTimers(now);

    // guard flag on
    calling_expired_timers_ = true;

    // safe to callback outside critical section
    for (const Timer* expired_timer : expired) {
        // do timeout callback
        expired_timer->run();
    }

    // guard flag off
    calling_expired_timers_ = false;

    reset(expired);
}

std::vector<Timer*> TimerContainer::getExpiredTimers(TimeAnchor now) {
    assert(timer_list_.size() == heap_size_);
    TimerList expired;
    while (heap_size_ > 0 && heapTop()->expiration() <= now) {
        expired.push_back(heapTop());
        heapPop();
    }

    // check again
    assert(timer_list_.size() == heap_size_);
    return expired;
}

void TimerContainer::reset(const TimerList& expired) {
    // delete all expired timers
    for (Timer* expired_timer : expired) {
        delete expired_timer;
    }

    if (heap_size_ > 0) {
        TimeAnchor next_expire = heapTop()->expiration();
        detail::resetTimerfd(timerfd_, next_expire);
    }
}

//* timer heap functions

Timer* TimerContainer::heapTop() {
    return timer_list_.front();
}

bool TimerContainer::isHeapEmpty() {
    assert(heap_size_ == timer_list_.size());
    return heap_size_ == 0;
}

void TimerContainer::heapAdd(Timer* timer) {
    if (!timer->inHeap()) {
        timer_list_.push_back(timer);
        timer->setPosInHeap(heap_size_++);
    }
    assert(timer->timerPos() == static_cast<int>(heap_size_ - 1));
    heapSiftUp(static_cast<size_t>(timer->timerPos()));
}

void TimerContainer::heapDel(Timer* timer) {
    assert(heap_size_ > 0);
    assert(timer->inHeap() && static_cast<size_t>(timer->timerPos()) < heap_size_);
    assert(heap_size_ == timer_list_.size());

    // get its position in heap vector
    size_t del_pos = static_cast<size_t>(timer->timerPos());

    if (--heap_size_ > 0) {
        // swap the last timer with the timer at del_pos
        {
            Timer* tmp = timer_list_[heap_size_];
            timer_list_[heap_size_] = timer_list_[del_pos];
            timer_list_[del_pos] = tmp;
        }
        timer_list_[del_pos]->setPosInHeap(del_pos);

        //* check if need to do SiftUp or SiftDown
        heapSiftDown(del_pos);
        heapSiftUp(del_pos);
    }

    // drop the deleted timer
    timer_list_.back()->setPosInHeap(detail::kInvalidHeapPosition);
    timer_list_.pop_back();
}

void TimerContainer::heapPop() {
    assert(heap_size_ > 0);
    assert(heap_size_ == timer_list_.size());
    assert(0 == heapTop()->timerPos());

    if (--heap_size_ > 0) {
        // swap the last timer with the top timer
        {
            Timer* tmp = timer_list_[heap_size_];
            timer_list_[heap_size_] = timer_list_[0];
            timer_list_[0] = tmp;
        }
        timer_list_[0]->setPosInHeap(detail::kTopHeapPosition);
        timer_list_[heap_size_]->setPosInHeap(detail::kInvalidHeapPosition);

        // conduct SiftDown
        heapSiftDown(0);

        // drop the deleted timer
        timer_list_.pop_back();
    } else {
        heapTop()->setPosInHeap(detail::kInvalidHeapPosition);
        timer_list_.pop_back();
    }
}

void TimerContainer::heapSiftUp(size_t heap_pos) {
    // top position, no need to SiftUp
    if (heap_pos == 0) { return; }

    size_t child_pos = heap_pos;
    size_t parent_pos = detail::heapNodeParent(heap_pos);

    do {
        // heap order is invalid. It's a min heap.
        if (detail::cmpTimer(timer_list_[child_pos], timer_list_[parent_pos])) {
            // swap parent and child
            {
                Timer* tmp = timer_list_[child_pos];
                timer_list_[child_pos] = timer_list_[parent_pos];
                timer_list_[parent_pos] = tmp;
            }
            timer_list_[child_pos]->setPosInHeap(child_pos);
            timer_list_[parent_pos]->setPosInHeap(parent_pos);

            // renew positions
            child_pos = parent_pos;
            parent_pos = detail::heapNodeParent(child_pos);
        } else {
            break;
        }
    } while (child_pos > 0);
}

void TimerContainer::heapSiftDown(size_t heap_pos) {
    size_t parent_pos = heap_pos;
    size_t child_pos = detail::heapNodeLeftChild(parent_pos);

    if (child_pos >= heap_size_) { return; }

    do {
        if ((1+child_pos) < heap_size_ &&
                (detail::cmpTimer(timer_list_[1+child_pos], timer_list_[child_pos]))) {
            // let child_pos be the position of the smaller child
            ++child_pos;
        }

        if (detail::cmpTimer(timer_list_[child_pos], timer_list_[parent_pos])) {
            // swap parent and child
            {
                Timer* tmp = timer_list_[child_pos];
                timer_list_[child_pos] = timer_list_[parent_pos];
                timer_list_[parent_pos] = tmp;
            }
            timer_list_[child_pos]->setPosInHeap(child_pos);
            timer_list_[parent_pos]->setPosInHeap(parent_pos);

            // renew positions
            parent_pos = child_pos;
            child_pos = detail::heapNodeLeftChild(parent_pos);
        } else {
            return;
        }
    } while (child_pos < heap_size_);
}

} // namespace net
} // namespace netlibcc