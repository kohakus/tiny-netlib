#include "netlibcc/net/EPoller.h"

#include <poll.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cassert>

#include "netlibcc/net/Channel.h"
#include "netlibcc/core/Logger.h"

// utilize POLL flags in epoll functions for compatibility
static_assert(EPOLLIN == POLLIN,        "epoll event flags are implemented identical to poll events");
static_assert(EPOLLERR == POLLERR,      "epoll event flags are implemented identical to poll events");
static_assert(EPOLLHUP == POLLHUP,      "epoll event flags are implemented identical to poll events");
static_assert(EPOLLPRI == POLLPRI,      "epoll event flags are implemented identical to poll events");
static_assert(EPOLLOUT == POLLOUT,      "epoll event flags are implemented identical to poll events");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll event flags are implemented identical to poll events");

namespace netlibcc {
namespace net {

EPoller::EPoller(EventLoop* loop)
    : owner_loop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      num_fd_(0),
      event_list_(kInitEventListSize) {
    if (epollfd_ < 0) {
        // if epoll_create return -1, the errno is set to indicate the error
        LOG_SYSFATAL << "EPoller::EPoller, error occur";
    }
}

EPoller::~EPoller() {
    // for RAII
    ::close(epollfd_);
}

void EPoller::poll(ChannelList* active_channels, int timeout) {
    LOG_TRACE << "fd count: " << num_fd_;

    int num_event = ::epoll_wait(epollfd_,
                                &*event_list_.begin(),
                                static_cast<int>(event_list_.size()),
                                timeout);

    // the return value of epoll_wait can be >0, 0, -1
    // if return -1, the errno can be EBADF, EFAULT, EINTR or EINVAL
    int curr_errno = errno;
    if (num_event < 0) {
        // ignore signal interrupt
        if (curr_errno != EINTR) {
            errno = curr_errno;
            LOG_SYSERR << "EPoller::poll()";
        }
    } else if (num_event == 0) {
        LOG_TRACE << "timeout and no file descriptor became ready";
    } else {
        LOG_TRACE << num_event << " events happened";
        fillActiveChannels(num_event, active_channels);
        if (event_list_.size() == static_cast<size_t>(num_event)) {
            event_list_.resize(event_list_.size() * 2);
        }
    }
}

void EPoller::fillActiveChannels(int num_event, ChannelList* active_channels) const {
    for (int i = 0; i < num_event; i++) {
        //* set the corresponding channel obj
        Channel* channel = static_cast<Channel*>(event_list_[i].data.ptr);
        channel->set_revents(event_list_[i].events);
        active_channels->push_back(channel);
    }
}

void EPoller::update(int operation, Channel* channel) {
    //* create and construct epoll_event for registering
    struct epoll_event channel_event;
    memset(&channel_event, 0, sizeof channel_event);
    // set interested events
    channel_event.events = channel->events();
    // set user data
    channel_event.data.ptr = channel;
    int fd = channel->fd();

    LOG_TRACE << "epoll_ctl op = " << opStr(operation) << " fd = " << fd;
    if (::epoll_ctl(epollfd_, operation, fd, &channel_event) < 0) {
        (operation == EPOLL_CTL_DEL ? (LOG_SYSERR) : (LOG_SYSFATAL)) << "epoll_ctl op = "
                                                                     << opStr(operation)
                                                                     << " fd = " << fd;
    }
}

void EPoller::updateChannel(Channel* channel) {
    // check if this is called in the corresponding IO thread
    owner_loop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd()
              << (channel->inPollar() ? " in EPollar" : " not in EPollar");

    if(!channel->inPollar()) {
        ++num_fd_;
        channel->setInPollarState(true);
        update(EPOLL_CTL_ADD, channel);
    } else {
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setInPollarState(false);
            --num_fd_;
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel* channel) {
    owner_loop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channel->isNoneEvent());
    if (channel->inPollar()) {
        update(EPOLL_CTL_DEL, channel);
        channel->setInPollarState(false);
        --num_fd_;
    }
}

const char* EPoller::opStr(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            // must be wrong when op is unrecognizable
            assert(false && "ERROR EPoll OP");
            return "Unknown Operation";
    }
}

} // namespace net
} // namespace netlibcc