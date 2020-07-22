#ifndef NETLIBCC_NET_IOTHREAD_H_
#define NETLIBCC_NET_IOTHREAD_H_

#include "netlibcc/core/Thread.h"

namespace netlibcc {
namespace net {

// forward declaration
class EventLoop;

class IOThread : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    IOThread(const ThreadInitCallback& cb = ThreadInitCallback(),
             const std::string& name = std::string());
    ~IOThread();

    EventLoop* start();

private:
    // the thread function to run loop
    void threadFunc();

    // flag of exiting state
    bool exiting_;

    Mutex mutex_;
    EventLoop* loop_;
    Thread thread_;
    Condition cond_;
    ThreadInitCallback init_callback_;
};

} // namespace net
} // namespace netlibcc


#endif // NETLIBCC_NET_IOTHREAD_H_