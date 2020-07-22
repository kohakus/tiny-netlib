#ifndef NETLIBCC_NET_EVENTLOOP_H_
#define NETLIBCC_NET_EVENTLOOP_H_

#include <atomic>
#include <vector>
#include <memory>
#include <functional>

#include "netlibcc/net/TimerId.h"
#include "netlibcc/core/Mutex.h"
#include "netlibcc/core/TimeAnchor.h"

namespace netlibcc {
namespace net {

// forward declarations
class Channel;
class EPoller;
class TimerContainer;

// reactor of one loop per thread
class EventLoop : Noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //* methods to check if work-thread-transfer strategy work correctly
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const {
        return thread_id_ == netlibcc::thisthread::tid();
    }

    // looping of EventLoop
    void loop();

    // quit the loop
    void quit();

    // wake up loop in a thread
    void wakeup();

    //* functions to use timer
    TimerId runAt(TimeAnchor time, std::function<void()> cb);
    TimerId runAfter(double delay, std::function<void()> cb);
    void cancelTimer(TimerId timerid);

    //* functions used to support work-thread-transfer strategy
    void runInLoop(Functor cb);
    void queInLoop(Functor cb);
    size_t queueSize() const;

    //* interface for linking channel and epoller
    void updateChannel(Channel*);
    void removeChannel(Channel*);

    //* access functions
    bool isEventHandeling() const { return event_handeling_; }
    bool hasChannel(Channel*);
    int64_t iteration() const { return iteration_; }

    static EventLoop* getCurrentThreadLoop();

private:
    // fatal error if the corresponding function is not called in created thread
    // this usually means that work-thread-transfer does not behaves as expected
    void abortNotInLoopThread();

    // call pending functors
    void doPendingTasks();

    // read from created eventfd (that is, wakeupfd_)
    void wakeupRead();

    // every fd corresponds to a Channel obj, hence we need ChannelList
    using ChannelList = std::vector<Channel*>;

    // !! Note that the declaration order of the following data members is very important.
    // !! thread_id_ must be initliazaed before the creation of epoller_ and timer_container_
    // !! the creation of epoller_ should be earlier than timer_container_

    //* work states
    const pid_t thread_id_;
    // can be quit by another thread
    std::atomic_bool quit_;
    int64_t iteration_;
    bool looping_;
    bool event_handeling_;
    bool calling_pending_functors_;

    //* components of event loop
    std::unique_ptr<EPoller> epoller_;
    std::unique_ptr<TimerContainer> timer_container_;

    //* used for eventloop wake up for calling callback
    int wakeupfd_;
    std::unique_ptr<Channel> wakeup_channel_;

    // the results from EPoll
    ChannelList active_channels_;

    //* functors transferred from other threads, which is proected by mutex
    mutable Mutex mutex_;
    std::vector<Functor> pending_functors_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_EVENTLOOP_H_