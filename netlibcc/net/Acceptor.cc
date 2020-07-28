#include "netlibcc/net/Acceptor.h"

#include <errno.h>
#include <unistd.h>

#include "netlibcc/net/EventLoop.h"
#include "netlibcc/base/InetAddr.h"
#include "netlibcc/base/SockOps.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {
namespace net {

const int Acceptor::kNumTryAccept = 10;

Acceptor::Acceptor(EventLoop* loop, const InetAddr& listen_addr, bool reuse_port)
            : loop_(loop),
              listen_socket_(sockets::create(listen_addr.family())),
              listen_channel_(loop, listen_socket_.fd()),
              listening_(false) {
    listen_socket_.setReuseAddr(true);
    listen_socket_.setReusePort(reuse_port);
    listen_socket_.bind(listen_addr);
    listen_channel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    listen_channel_.disableAll();
    listen_channel_.remove();

    // no need to close listen_socket_ explicitly
    // it is managed by RAII Socket
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    listen_socket_.listen();
    listen_channel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();

    // every time listen socket is readable, try to accept some new conn
    for (int try_it = 0; try_it < kNumTryAccept; try_it++) {
        InetAddr peer_addr;
        int connfd = listen_socket_.accept(&peer_addr);
        if (connfd < 0) {
            if (errno == EAGAIN) {
                LOG_TRACE << "Acceptor::handleRead() break for EAGAIN, iter = " << try_it + 1;
                break;
            } else {
                LOG_SYSERR << "in Acceptor::handleRead";
            }
        } else {
            if (new_conn_callback_) {
                new_conn_callback_(connfd, peer_addr);
            } else {
                sockets::close(connfd);
            }
        }
    }

    // TODO: deal with EMFILE
    // the per-process descriptor may be exhausted
}

} // namespace net
} // namespace netlibcc