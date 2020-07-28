#ifndef NETLIBCC_NET_ACCEPTOR_H_
#define NETLIBCC_NET_ACCEPTOR_H_

#include "netlibcc/net/Channel.h"
#include "netlibcc/base/Socket.h"

namespace netlibcc {
namespace net {

// forward declarations
class EventLoop;
class InetAddr;

// an inner class used by TCPServer
class Acceptor : Noncopyable {
public:
    // the user (TCPServer) can define callback function like this:
    // void newConnection(int conn_sockfd, const InetAddr& peer_addr);
    // here we need not to convert socketfd to Socket obj. The corresponding
    // socketfd will be taken and controlled by Socket obj in TCPConnection at last
    using NewConnCallback = std::function<void (int, const InetAddr&)>;

    Acceptor(EventLoop* loop, const InetAddr& listen_addr, bool reuse_port);
    ~Acceptor();

    void setNewConnCallback(const NewConnCallback& cb) {
        new_conn_callback_ = cb;
    }

    // get listen socket, as well as enabling read event of the corresponding Channel
    void listen();

    // state access function
    bool isListening() const {
        return listening_;
    }

private:
    // readable event callback
    void handleRead();

    EventLoop* loop_;

    //* Acceptor should hold a listen socket fd
    Socket listen_socket_;
    Channel listen_channel_;

    NewConnCallback new_conn_callback_;

    // Acceptor state
    bool listening_;

    // the number of times handeRead try to accept
    static const int kNumTryAccept;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_ACCEPTOR_H_