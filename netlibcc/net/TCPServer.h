#ifndef NETLIBCC_NET_TCPSERVER_H_
#define NETLIBCC_NET_TCPSERVER_H_

#include "netlibcc/net/TCPConnection.h"

#include <atomic>
#include <map>

namespace netlibcc {
namespace net {

// forward declarations
class Acceptor;
class EventLoop;
class IOThreadPool;

// interface class that is directly used by users
class TCPServer : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    TCPServer(EventLoop* loop,
              const InetAddr& listen_addr,
              const std::string& name,
              bool reuse_port = false);
    ~TCPServer();

    //* access functions
    const std::string& getIpPortStr() const { return ip_port_; }
    const std::string& getName() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    //* for using IOThreadPool
    void setThreadNum(int num);
    void setThreadInitCallback(const ThreadInitCallback& cb);
    void start();

    //* callback functions setting (for TCPConnection)
    void setConnectionCallback(const ConnectionCallback& cb) {
        conn_cb_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        message_cb_ = cb;
    }

private:
    // the entry point of read callback which is set in Acceptor, only run in Acceptor loop thread
    // this function works as callback of the Acceptor for creating and setting new TCPConnection
    void newConnection(int conn_sockfd, const InetAddr& peer_addr);

    // the entry point of close callback in Channel
    void removeConnection(const TCPConnectionPtr& conn);
    void removeConnectionInLoop(const TCPConnectionPtr& conn);

    // the Acceptor loop
    EventLoop* loop_;

    //* accompanying data members
    const std::string ip_port_;
    const std::string name_;

    //* members for inner use
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<IOThreadPool> thread_pool_;

    //* callback functions for TCPConnection setting
    ConnectionCallback conn_cb_;
    MessageCallback message_cb_;

    // callback function of IOThread
    ThreadInitCallback thread_init_cb_;

    //* flags
    std::atomic_bool started_;
    int next_conn_id_;

    // storing conn_name->conn mapping
    std::map<std::string, TCPConnectionPtr> conn_map_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_TCPSERVER_H_