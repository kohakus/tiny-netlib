#ifndef NETLIBCC_NET_IOTHREADPOOL_H_
#define NETLIBCC_NET_IOTHREADPOOL_H_

#include <memory>
#include <vector>
#include <functional>

#include "netlibcc/core/Noncopyable.h"
#include "netlibcc/core/StringArg.h"

namespace netlibcc {
namespace net {

// forward declarations
class EventLoop;
class IOThread;

class IOThreadPool : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    IOThreadPool(EventLoop* base_loop, StringArg name);
    ~IOThreadPool();

    // set the number of threads in pool
    void setThreadNum(int num);

    // create N threads in the pool
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // for dispatching conn socket fd
    EventLoop* getNextLoop();

    //* access functions
    bool isStarted() const { return started_; }
    const std::string& name() const { return name_; }

private:
    // base loop of TCPServer
    EventLoop* base_loop_;

    std::string name_;
    int num_threads_;
    int next_;
    bool started_;
    std::vector<std::unique_ptr<IOThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_IOTHREADPOOL_H_