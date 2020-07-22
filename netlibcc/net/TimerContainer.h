#ifndef NETLIBCC_NET_TIMERCONTAINER_H_
#define NETLIBCC_NET_TIMERCONTAINER_H_

#include <vector>

#include "netlibcc/net/Channel.h"
#include "netlibcc/core/TimeAnchor.h"
#include "netlibcc/core/Noncopyable.h"

namespace netlibcc {
namespace net {

// forward declarations
class EventLoop;
class Timer;
class TimerId;

class TimerContainer : Noncopyable {
public:
    using TimerCallback = std::function<void()>;

    explicit TimerContainer(EventLoop* loop);
    ~TimerContainer();

    // add a Timer to TimerContainer
    TimerId addTimer(TimerCallback cb,
                     TimeAnchor expiration,
                     double interval);

    // delete a Timer from TimerContainer
    void cancel(TimerId timerid);

private:
    using TimerList = std::vector<Timer*>;

    //* work-thread-transfer strategy
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerid);

    // called when timerfd alarms
    void handleRead();

    // move out expired timers from timer container
    TimerList getExpiredTimers(TimeAnchor now);

    // delete all expired timers and reset timerfd
    void reset(const TimerList& expired);

    //* timer heap functions
    Timer* heapTop();
    bool isHeapEmpty();
    void heapAdd(Timer* timer);
    void heapDel(Timer* timer);
    void heapSiftUp(size_t heap_pos);
    void heapSiftDown(size_t heap_pos);
    void heapPop();
    // Push is alias of Add
    void heapPush(Timer* timer) {
        heapAdd(timer);
    }

    // inner Timer container
    TimerList timer_list_;

    // timer heap size
    size_t heap_size_;

    //* data members
    EventLoop* loop_;
    // all timeout tasks shared a same timerfd
    const int timerfd_;
    // the Channel to manage timerfd_
    Channel timerfd_channel_;
    // for cancel
    bool calling_expired_timers_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_TIMERCONTAINER_H_