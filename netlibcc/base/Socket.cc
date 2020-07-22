#include "netlibcc/base/Socket.h"

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <cstdio>

#include "netlibcc/base/InetAddr.h"
#include "netlibcc/base/SockOps.h"
#include "netlibcc/core/Logger.h"

namespace netlibcc {
namespace net {

Socket::~Socket() {
    sockets::close(sockfd_);
}

void Socket::listen() {
    sockets::listen(sockfd_);
}

void Socket::bind(const InetAddr& local_addr) {
    sockets::bind(sockfd_, local_addr.getSockAddr());
}

int Socket::accept(InetAddr* peer_addr) {
    // address that return from accept
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peer_addr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    sockets::shutdownWR(sockfd_);
}

// Socket option setting
bool Socket::setTcpNoDelay(bool on) {
    int val = on ? 1 : 0;
    return ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &val,
                        static_cast<socklen_t>(sizeof val)) < 0 ? false : true;
}

bool Socket::setReuseAddr(bool on) {
    int val = on ? 1 : 0;
    return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &val,
                        static_cast<socklen_t>(sizeof val)) < 0 ? false : true;
}

bool Socket::setReusePort(bool on) {
    int val = on ? 1 : 0;
    return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &val,
                        static_cast<socklen_t>(sizeof val)) < 0 ? false : true;
}

bool Socket::setKeepAlive(bool on) {
    int val = on ? 1 : 0;
    return ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &val,
                        static_cast<socklen_t>(sizeof val)) < 0 ? false : true;
}

} // namespace net
} // namespace netlibcc