#include "netlibcc/net/EventLoop.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include "netlibcc/net/EPoller.h"
#include "netlibcc/net/TimerContainer.h"
#include "netlibcc/base/IgnoreSigPipe.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {
namespace net {
namespace {

// TLS data for remember which loop the thread has
__thread EventLoop* t_loopInThisThread = 0;

// the number of microseconds epoll_wait holds
const int kEPollWaitTime = 10000;

// ignore SIGPIPE signal
netlibcc::net::detail::IgnoreSigPipe ignore_sigpipe_init_obj;

} // anonymous namespace

namespace detail {

int createEventfd() {
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfd < 0) {
        LOG_SYSERR << "Failed in eventfd create";
    }
    return eventfd;
}

} // namespace detail

EventLoop::EventLoop()
        : thread_id_(netlibcc::thisthread::tid()),
          quit_(false),
          iteration_(0),
          looping_(false),
          event_handeling_(false),
          calling_pending_functors_(false),
          epoller_(new EPoller(this)),
          timer_container_(new TimerContainer(this)),
          wakeupfd_(detail::createEventfd()),
          wakeup_channel_(new Channel(this, wakeupfd_)),
          active_channels_(),
          mutex_(),
          pending_functors_() {
    LOG_DEBUG << "EventLoop created " << (void*)this << " in thread " << thread_id_;

    // make sure one loop per thread
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << (void*)t_loopInThisThread
                  << " already exists in thread " << thread_id_;
    } else {
        t_loopInThisThread = this;
    }

    wakeup_channel_->setReadCallback(std::bind(&EventLoop::wakeupRead, this));
    wakeup_channel_->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << (void*)this << " of thread " << thread_id_
              << " destructs in thread " << netlibcc::thisthread::tid();

    wakeup_channel_->disableAll();
    wakeup_channel_->remove();
    ::close(wakeupfd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();

    // renew states
    looping_ = true;
    quit_ = false;
    LOG_TRACE << "EventLoop " << (void*)this << " start looping";

    // main event loop
    while (!quit_) {
        active_channels_.clear();
        epoller_->poll(&active_channels_, kEPollWaitTime);

        // epoll return. Increasing the iteration counter
        ++iteration_;

        //* deal with callbacks
        // guard flag on
        event_handeling_ = true;

        for (Channel* channel : active_channels_) {
            channel->handleEvent();
        }

        // guard flag off
        event_handeling_ = false;

        // do transfered functor tasks
        doPendingTasks();
    }

    LOG_TRACE << "EventLoop " << (void*)this << " stop looping";
    // renew states
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

// wakeup the corresponding loop (which sleep on EPoll) immediately
void EventLoop::wakeup() {
    uint64_t arbitrary = 1;
    ssize_t n = ::write(wakeupfd_, &arbitrary, sizeof arbitrary);
    if (n != sizeof arbitrary) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::wakeupRead() {
    uint64_t howmany;
    ssize_t n = ::read(wakeupfd_, &howmany, sizeof howmany);
    if (n != sizeof howmany) {
        LOG_ERROR << "EventLoop::wakeupRead() reads " << n << " bytes instead of 8";
    }
}

TimerId EventLoop::runAt(TimeAnchor time, std::function<void()> cb) {
    return timer_container_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, std::function<void()> cb) {
    int64_t delta = static_cast<int64_t>(delay * TimeAnchor::kMicroSecondsPerSecond);
    TimeAnchor time(TimeAnchor::now().timeMicro() + delta);
    return runAt(time, std::move(cb));
}

void EventLoop::cancelTimer(TimerId timerid) {
    return timer_container_->cancel(timerid);
}

//* functions used to support work-thread-transfer strategy

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queInLoop(cb);
    }
}

void EventLoop::queInLoop(Functor cb) {
    // pending_functors_ can be seen by multiple threads
    {
        MutexLockGuard lock(mutex_);
        pending_functors_.push_back(std::move(cb));
    }
    if (!isInLoopThread() || calling_pending_functors_) {
        wakeup();
    }
}

size_t EventLoop::queueSize() const {
    MutexLockGuard lock(mutex_);
    return pending_functors_.size();
}

//* interface for linking channel and epoller

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return channel->inPollar();
}

void EventLoop::doPendingTasks() {
    // create a vector for swap
    std::vector<Functor> functors;

    // guard flag on
    calling_pending_functors_ = true;

    // swap pending functors, for shrinking critical section
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pending_functors_);
    }

    // deal with pending functors safely
    for (const Functor& functor : functors) {
        functor();
    }

    // guard flag off
    calling_pending_functors_ = false;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "abortNotInLoopThread() EventLoop " << (void*)this << " was created in tid = "
              << thread_id_ << ", current thread id = " << netlibcc::thisthread::tid();
}

EventLoop* EventLoop::getCurrentThreadLoop() {
    return t_loopInThisThread;
}

} // namespace net
} // namespace netlibcc