#include "netlibcc/net/TCPServer.h"

#include "netlibcc/net/Acceptor.h"
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/net/IOThreadPool.h"
#include "netlibcc/base/SockOps.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {

using std::placeholders::_1;
using std::placeholders::_2;

namespace net {

void defaultConnectionCallback(const TCPConnectionPtr& conn) {
    LOG_TRACE << conn->sockAddr().toIpPortStr() << " -> "
              << conn->peerAddr().toIpPortStr() << " is "
              << (conn->isConnected() ? "connected" : "down");
}

void defaultMessageCallback(const TCPConnectionPtr&, Buffer* buf) {
    // message callback is called after data is read in buffer
    buf->retrieveAll();
}

TCPServer::TCPServer(EventLoop* loop,
                     const InetAddr& listen_addr,
                     const std::string& name,
                     bool reuse_port)
                : loop_(loop),
                  ip_port_(listen_addr.toIpPortStr()),
                  name_(name),
                  acceptor_(new Acceptor(loop, listen_addr, reuse_port)),
                  thread_pool_(new IOThreadPool(loop, name_)),
                  conn_cb_(defaultConnectionCallback),
                  message_cb_(defaultMessageCallback),
                  started_(false),
                  next_conn_id_(1) {
    acceptor_->setNewConnCallback(std::bind(&TCPServer::newConnection, this, _1, _2));
}

TCPServer::~TCPServer() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TCPServer::~TCPServer " << name_;

    // destroy all connections
    for (auto& conn_item : conn_map_) {
        TCPConnectionPtr conn(conn_item.second);
        // just release the TCPConnection reference, instead of
        // erase the whole item directly.
        conn_item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TCPConnection::afterDestroyed, conn));
        // The socketfd is automatically closed by RAII Socket class' dtor
    }
}

void TCPServer::setThreadNum(int num) {
    thread_pool_->setThreadNum(num);
}

void TCPServer::setThreadInitCallback(const ThreadInitCallback& cb) {
    thread_init_cb_ = cb;
}

void TCPServer::start() {
    if (!started_.exchange(true)) {
        thread_pool_->start(thread_init_cb_);
        assert(!acceptor_->isListening());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TCPServer::newConnection(int conn_sockfd, const InetAddr& peer_addr) {
    loop_->assertInLoopThread();

    // round-robin
    EventLoop* selected_loop = thread_pool_->getNextLoop();
    std::string conn_name = name_ + fmt::format("-{}#{}", ip_port_, next_conn_id_++);

    LOG_INFO << "TCPServer::newConnection " << conn_name << " from " << peer_addr.toIpPortStr();

    // create new TCPConnection obj to manage conn_sockfd
    TCPConnectionPtr conn = std::make_shared<TCPConnection>(selected_loop,
                                                            conn_name,
                                                            conn_sockfd,
                                                            InetAddr(sockets::getSockAddr(conn_sockfd)),
                                                            peer_addr);

    // add new conn_item
    conn_map_[conn_name] = conn;

    //* set callbacks
    conn->setConnectionCallback(conn_cb_);
    conn->setMessageCallback(message_cb_);
    conn->setCloseCallback(std::bind(&TCPServer::removeConnection, this, _1));
    selected_loop->runInLoop(std::bind(&TCPConnection::afterEstablished, conn));
}

// this function is called in conn_loop
void TCPServer::removeConnection(const TCPConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TCPServer::removeConnectionInLoop, this, conn));
}

void TCPServer::removeConnectionInLoop(const TCPConnectionPtr& conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TCPServer::removeConnectionInLoop" << " - connection " << conn->name();
    size_t n = conn_map_.erase(conn->name());
    assert(n == 1);
    EventLoop* conn_loop = conn->getLoop();

    // Note that the queInLoop here is necessary to prolong the life of TCPConnection conn
    conn_loop->queInLoop(std::bind(&TCPConnection::afterDestroyed, conn));
}

} // namespace net
} // namespace netlibcc