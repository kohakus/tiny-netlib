#ifndef NETLIBCC_NET_CHANNEL_H_
#define NETLIBCC_NET_CHANNEL_H_

#include <functional>
#include <memory>

#include "netlibcc/core/Noncopyable.h"

namespace netlibcc {
namespace net {

// forward declaration of event loop
class EventLoop;

// the class Channel manages the event dispatch that related a specific fd
class Channel : Noncopyable {
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    // key operation of Channel
    // the actual work is done by handleEventCallback()
    void handleEvent();

    //* callback setting functions
    void setReadCallback(EventCallback cb) {
        readCallback_ = std::move(cb);
    }

    void setWriteCallback(EventCallback cb) {
        writeCallback_ = std::move(cb);
    }

    void setCloseCallback(EventCallback cb) {
        closeCallback_ = std::move(cb);
    }

    void setErrorCallback(EventCallback cb) {
        errorCallback_ = std::move(cb);
    }

    //* access functions
    int fd() const { return fd_; }
    int events() const { return sevents_; }
    bool inPollar() const { return in_pollar_; }
    EventLoop* ownerLoop() { return loop_; }

    //* checking the current interested events
    bool isNoneEvent() const {
        return sevents_ == kNoneEvent;
    }

    bool isWritingEvent() const {
        return sevents_ & kWriteEvent;
    }

    bool isReadingEvent() const {
        return sevents_ & kReadEvent;
    }

    //* setting the interested events
    void enableReading() {
        sevents_ |= kReadEvent;
        update();
    }

    void enableWriting() {
        sevents_ |= kWriteEvent;
        update();
    }

    //* delete interested events
    void disableReading() {
        sevents_ &= ~kReadEvent;
        update();
    }

    void disableWriting() {
        sevents_ &= ~kWriteEvent;
        update();
    }

    void disabelAll() {
        sevents_ = kNoneEvent;
        update();
    }

    void tie(const std::shared_ptr<void>&);

    // Channel::update() -> EventLoop::updateChannel() -> Poller::updateChannel()
    void update();
    // Channel::remove() -> EventLoop::removeChannel() -> Poller::removeChannel()
    void remove();

    // used by Poller, to get the events happend
    void set_revents(int revents) {
        revents_ = revents;
    }

    void setInPollarState(bool state) {
        in_pollar_ = state;
    }

private:
    // the function that dispatch callback functions according to the event type
    void handleEventCallback();

    //* some events that the IO Multiplexing may interested
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    //* data members
    EventLoop* loop_;
    const int fd_;
    int sevents_;
    // set by Poller
    int revents_;
    // let the Channel obj remember its state whether it is considered by Poller
    bool in_pollar_;

    //* weak callback states
    std::weak_ptr<void> tie_;
    bool tied_;
    bool event_handeling_;
    bool added_to_loop_;

    //* callback tasks
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_CHANNEL_H_