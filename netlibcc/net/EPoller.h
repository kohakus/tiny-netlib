#ifndef NETLIBCC_NET_EPOLLER_H_
#define NETLIBCC_NET_EPOLLER_H_

#include <vector>
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/core/Noncopyable.h"

// ********** (From Linux Manual Page) *********
//    typedef union epoll_data {
//        void        *ptr;
//        int          fd;
//        uint32_t     u32;
//        uint64_t     u64;
//    } epoll_data_t;
//
//    struct epoll_event {
//        uint32_t     events;  /* Epoll events */
//        epoll_data_t data;    /* User data variable */
//    };
// ********** (From Linux Manual Page) *********

// forward declaration of epoll_event
struct epoll_event;

namespace netlibcc {
namespace net {

// forward declaration of Channel
class Channel;

// Epoll IO Multiplexing for EventLoop
class EPoller : Noncopyable {
public:
    // every fd corresponds to a Channel obj, hence we need ChannelList
    // every EventLoop should hold a ChannelList for Channel management
    using ChannelList = std::vector<Channel*>;

    EPoller(EventLoop* loop);
    ~EPoller();

    // the wrapper of epoll_wait(2)
    void poll(ChannelList* active_channels, int timeout);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    static const int kInitEventListSize = 16;

    // show a specific epoll operation
    static const char* opStr(int op);

    // set active channels to ChannelList Channel objs
    void fillActiveChannels(int num_event, ChannelList* active_channels) const;

    // the wrapper of epoll_ctl(2), it also set channel as user data
    void update(int operation, Channel* channel);

    // owner
    EventLoop* owner_loop_;

    // epollfd created by epoll_create(2)
    int epollfd_;

    // the number of fd that the EPoller watching currently
    int num_fd_;

    // used for storing the return results of epoll_wait(2)
    std::vector<struct epoll_event> event_list_;

    // Note that if we use epoll, we need not to hold a mapping from
    // fd to its corresponding channel, for the epoll_ctl(2) will add
    // this link naturally when the event table is updated.
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_EPOLLER_H_