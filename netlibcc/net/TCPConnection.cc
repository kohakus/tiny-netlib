#include "netlibcc/net/TCPConnection.h"

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "netlibcc/net/Channel.h"
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/base/Socket.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {
namespace net {

TCPConnection::TCPConnection(EventLoop* loop,
                             const std::string& name,
                             int conn_sockfd,
                             const InetAddr& sock_addr,
                             const InetAddr& peer_addr)
                : loop_(loop),
                  name_(name),
                  conn_state_(kConnecting),
                  isreading_(true),
                  conn_socket_(new Socket(conn_sockfd)),
                  conn_channel_(new Channel(loop, conn_sockfd)),
                  sock_addr_(sock_addr),
                  peer_addr_(peer_addr) {
    //* set channel callback functions
    conn_channel_->setReadCallback(std::bind(&TCPConnection::handleRead, this));
    conn_channel_->setWriteCallback(std::bind(&TCPConnection::handleWrite, this));
    conn_channel_->setCloseCallback(std::bind(&TCPConnection::handleClose, this));
    conn_channel_->setErrorCallback(std::bind(&TCPConnection::handleError, this));

    LOG_DEBUG << "TCPConnection::TCPConnection " << name_ << " fd = " << conn_socket_->fd();

    // set TCP KeepAlive by default
    conn_socket_->setKeepAlive(true);
}

TCPConnection::~TCPConnection() {
    LOG_DEBUG << "TCPConnection::~TCPConnection " << name_ << " fd = " << conn_channel_->fd()
              << " state = " << conn_state_;
    assert(conn_state_ == kDisconnected);
}

void TCPConnection::send(const void* message, int len) {
    send(std::string(static_cast<const char*>(message), len));
}

void TCPConnection::send(const char* message, int len) {
    send(std::string(message, len));
}

// FIXME: More efficient implementation using move or forward
void TCPConnection::send(const std::string& message) {
    if (conn_state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            void(TCPConnection::*fp)(const std::string&) = &TCPConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, message));
        }
    }
}

void TCPConnection::send(Buffer* buf) {
    if (conn_state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->rpos(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            void(TCPConnection::*fp)(const std::string&) = &TCPConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsStr()));
        }
    }
}

// FIXME: More efficient implementation
void TCPConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.c_str(), message.size());
}

void TCPConnection::sendInLoop(const void* message, size_t len) {
    loop_->assertInLoopThread();
    if (conn_state_ == kDisconnected) {
        LOG_WARN << "try to send to disconnected conn_fd, give up";
        return;
    }
    wt_buffer_.append(static_cast<const char*>(message), len);
    if (!conn_channel_->isWritingEvent()) {
        conn_channel_->enableWriting();
    }
}

void TCPConnection::shutdown() {
    if (conn_state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TCPConnection::shutdownInLoop, this));
    }
}

void TCPConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!conn_channel_->isWritingEvent()) {
        conn_socket_->shutdownWrite();
    }
}

void TCPConnection::setTCPNoDelay(bool on) {
    conn_socket_->setTcpNoDelay(on);
}

void TCPConnection::setReadState(bool enable) {
    loop_->runInLoop(std::bind(&TCPConnection::setReadStateInLoop, this, enable));
}

void TCPConnection::setReadStateInLoop(bool enable) {
    loop_->assertInLoopThread();
    if(enable) {
        if (!isreading_ || !conn_channel_->isReadingEvent()) {
            conn_channel_->enableReading();
            isreading_ = true;
        }
    } else {
        if (isreading_ || conn_channel_->isReadingEvent()) {
            conn_channel_->disableReading();
            isreading_ = false;
        }
    }
}

void TCPConnection::afterEstablished() {
    loop_->assertInLoopThread();
    assert(conn_state_ == kConnecting);
    setState(kConnected);
    conn_channel_->tie(shared_from_this());
    conn_channel_->enableReading();

    // do user connection callback function
    conn_cb_(shared_from_this());
}

void TCPConnection::afterDestroyed() {
    loop_->assertInLoopThread();
    if (conn_state_ == kConnected) {
        setState(kDisconnected);
        conn_channel_->disableAll();
        conn_cb_(shared_from_this());
    }
    conn_channel_->remove();
}

// when conn_sockfd is readable
void TCPConnection::handleRead() {
    loop_->assertInLoopThread();
    int err_info = 0;
    ssize_t n = rd_buffer_.readFromFd(conn_channel_->fd(), err_info);

    if (n > 0) {
        // case1: message arrived
        message_cb_(shared_from_this(), &rd_buffer_);
    } else if (n == 0) {
        // case2: client is closed
        handleClose();
    } else {
        // case3: error occurred
        errno = err_info;
        LOG_SYSERR << "TCPConnection::handleRead";
        handleError();
    }
}

// when conn_sockfd is writable
void TCPConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (conn_channel_->isWritingEvent()) {
        // write buffer data to conn_sockfd
        ssize_t n = ::write(conn_channel_->fd(),
                            wt_buffer_.rpos(),
                            wt_buffer_.readableBytes());

        if (n > 0) {
            wt_buffer_.retrieve(n);
            if (wt_buffer_.readableBytes() == 0) {
                // once all data is written completely, stop observing writable
                // event, for preventing busy loop
                conn_channel_->disableWriting();
                if (conn_state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_SYSERR << "TCPConnection::handleWrite";
        }

    } else {
        LOG_TRACE << "Connection fd = " << conn_channel_->fd()
                  << " is down, no more writing";
    }
}

void TCPConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "fd = " << conn_channel_->fd() << " state = " << conn_state_;

    // the state should not be disconnected before closed
    assert(conn_state_ == kConnected || conn_state_ == kDisconnecting);
    setState(kDisconnected);
    conn_channel_->disableAll();

    TCPConnectionPtr life_guard(shared_from_this());
    conn_cb_(life_guard);
    close_cb_(life_guard);
}

void TCPConnection::handleError() {
    // optval
    int sock_err_info;
    socklen_t optlen = static_cast<socklen_t>(sizeof sock_err_info);

    if (::getsockopt(conn_channel_->fd(), SOL_SOCKET, SO_ERROR, &sock_err_info, &optlen) < 0) {
        LOG_SYSERR << "TCPConnection::handleError";
    } else {
        LOG_ERROR << "TCPConnection::handleError " << name_ << " SO_ERROR = " << sock_err_info;
    }
}

} // namespace net
} // namespace netlibcc