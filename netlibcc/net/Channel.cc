#include "netlibcc/net/Channel.h"
#include <poll.h>
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {
namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

// ctor
Channel::Channel(EventLoop* loop, int fd)
        : loop_(loop),
          fd_(fd),
          sevents_(0),
          revents_(0),
          in_pollar_(false),
          tied_(false),
          event_handeling_(false),
          added_to_loop_(false) {}

// dtor
Channel::~Channel() {
    assert(!event_handeling_);
    assert(!added_to_loop_);
}

void Channel::handleEvent() {
    // for weak callback
    std::shared_ptr<void> promotion;
    if (tied_) {
        promotion = tie_.lock();
        // check if the obj pointed by tie_ is still alive
        if (promotion) {
            handleEventCallback();
        }
    } else {
        handleEventCallback();
    }
}

void Channel::handleEventCallback() {
    // guard on
    event_handeling_ = true;

    //* deal with various events

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (closeCallback_) { closeCallback_(); }
    }

    if (revents_ & POLLNVAL) {
        LOG_WARN << "fd " << fd_ << " Channel::handleEvent() POLLNVAL";
    }

    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) { errorCallback_(); }
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) { readCallback_(); }
    }

    if (revents_ & POLLOUT) {
        if (writeCallback_) { writeCallback_(); }
    }

    // guard off
    event_handeling_ = false;
}

void Channel::tie(const std::shared_ptr<void>& tie_obj) {
    tie_ = tie_obj;
    tied_ = true;
}

void Channel::update() {
    added_to_loop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    assert(isNoneEvent());
    added_to_loop_ = false;
    loop_->removeChannel(this);
}

} // net
} // namespace netlibcc