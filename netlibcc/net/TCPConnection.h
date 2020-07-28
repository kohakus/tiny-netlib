#ifndef NETLIBCC_NET_TCPCONNECTION_H_
#define NETLIBCC_NET_TCPCONNECTION_H_

#include "netlibcc/base/Buffer.h"
#include "netlibcc/base/InetAddr.h"
#include "netlibcc/core/TimeAnchor.h"
#include "netlibcc/core/Noncopyable.h"

#include <memory>
#include <functional>

namespace netlibcc {
namespace net {

//* forward declarations
class Channel;
class EventLoop;
class Socket;
class TCPConnection;

//* aliases as interface
using TCPConnectionPtr   = std::shared_ptr<TCPConnection>;
using TimerCallback      = std::function<void ()>;
using ConnectionCallback = std::function<void (const TCPConnectionPtr&)>;
using CloseCallback      = std::function<void (const TCPConnectionPtr&)>;
using MessageCallback    = std::function<void (const TCPConnectionPtr&, Buffer*)>;

// interface class for managing connected socket fd
// can be hold by both users and inner class, using shared_ptr to manage it
class TCPConnection : Noncopyable,
                      public std::enable_shared_from_this<TCPConnection> {
public:
    TCPConnection(EventLoop* loop,
                  const std::string& name,
                  int conn_sockfd,
                  const InetAddr& sock_addr,
                  const InetAddr& peer_addr);
    ~TCPConnection();

    //* access functions
    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddr& sockAddr() const { return sock_addr_; }
    const InetAddr& peerAddr() const { return peer_addr_; }

    //* get current state
    bool isConnected() const { return conn_state_ == kConnected; }
    bool isDisconnected() const { return conn_state_ == kDisconnected; }

    // IO active write operations
    void send(const void* message, int len);
    void send(const char* message, int len);
    void send(const std::string& message);
    void send(Buffer* buf);

    // TCP-Half close
    void shutdown();

    //TODO: TCP active close

    //* set states
    void setTCPNoDelay(bool on);
    void setReadState(bool enable);

    //* set callbacks (used by TCPServer)
    void setConnectionCallback(const ConnectionCallback& cb) { conn_cb_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { message_cb_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { close_cb_ = cb; }

    //* functions as finishing touches, they should be called only once
    void afterEstablished();
    void afterDestroyed();

private:
    // 1. The TCPConnection obj is created in TCPServer after conn_sockfd is acquired,
    //    hence the initial state of TCPConnection is kConnecting.
    // 2. Once the TCPConnection obj is initialized, its state is set to kConnected by
    //    TCPConnection::afterEstablished in loop.
    // 3. IO operations are allowed during the state is kConnected.
    // 4. TCP-Half close shutdown will make state kDisconnecting, the write end is closed.
    // 5. the state should be kDisconnected if connection closed completely.
    enum ConnState { kDisconnected, kConnecting, kConnected, kDisconnecting };

    //* channel callbacks
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    //* InLoop auxiliary functions
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    void setReadStateInLoop(bool enable);

    void setState(ConnState state) {
        conn_state_ = state;
    }

    EventLoop* loop_;
    std::string name_;

    //* states data
    ConnState conn_state_;
    bool isreading_;

    //* connected socket fd management
    std::unique_ptr<Socket> conn_socket_;
    std::unique_ptr<Channel> conn_channel_;

    //* accompanying socket address
    const InetAddr sock_addr_;
    const InetAddr peer_addr_;

    //* callbacks set by TCPServer
    ConnectionCallback conn_cb_;
    MessageCallback message_cb_;
    CloseCallback close_cb_;

    //* read/write Buffers working with NIO
    Buffer rd_buffer_;
    Buffer wt_buffer_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_TCPCONNECTION_H_