#include "netlibcc/base/SockOps.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "netlibcc/base/SockUtils.h"
#include "netlibcc/core/Logger.h"


namespace netlibcc {
namespace net {
namespace sockets {

// set socket file flags by using fcntl
int setNonBlockAndCloseOnExec(int socketfd) {
    // setting fd to non-blocking
    int flags = ::fcntl(socketfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(socketfd, F_SETFL, flags);
    // if error occur, ret should be -1
    if (ret < 0) {
        LOG_ERROR << "sockets::setNonBlockAndCloseOnExec";
    }

    // setting fd to close-on-exec
    flags = ::fcntl(socketfd, F_GETFL, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(socketfd, F_SETFL, flags);
    if (ret < 0) {
        LOG_ERROR << "sockets::setNonBlockAndCloseOnExec";
    }
    return ret;
}

void bind(int socketfd, const struct sockaddr* addr) {
    int ret = ::bind(socketfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::bind";
    }
}

void listen(int socketfd) {
    // set backlog to SOMAXCONN
    int ret = ::listen(socketfd, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::listen";
    }
}

int accept(int socketfd, struct sockaddr* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    // get a connected socket fd
    int connfd = ::accept(socketfd, addr, &addrlen);
    if (connfd < 0) {
        //int err = errno;
        LOG_SYSERR << "sockets::accept";
        // TODO: add more detailed checking of errno
    }
    setNonBlockAndCloseOnExec(connfd);
    return connfd;
}

int accept(int socketfd, struct sockaddr_in* addr4) {
    return accept(socketfd, sockaddr_cast(addr4));
}

void close(int socketfd) {
    if (::close(socketfd) < 0) {
        LOG_SYSERR << "sockets::close";
    }
}

void shutdownWR(int socketfd) {
    if (::shutdown(socketfd, SHUT_WR) < 0) {
        LOG_SYSERR << "sockets::shutdownWR";
    }
}

void shutdownRD(int socketfd) {
    if (::shutdown(socketfd, SHUT_RD) < 0) {
        LOG_SYSERR << "sockets::shutdownRD";
    }
}

} // namespace sockets
} // namespace net
} // namespace netlibcc