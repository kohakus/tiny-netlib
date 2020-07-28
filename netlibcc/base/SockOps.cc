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

// create non-blocking TCP socket fd
// TODO: add IPv6 support
int create(sa_family_t family) {
    int socketfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (socketfd < 0) {
        LOG_SYSFATAL << "sockets::create";
    }
    setNonBlockAndCloseOnExec(socketfd);
    return socketfd;
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
        int err_info = errno;

        // TODO: add more detailed checking of errno
        // https://man7.org/linux/man-pages/man2/accept.2.html
        switch (err_info) {
            //* cases we just ignore the errno

            // not an accept error, just no need to accept any connections
            case EAGAIN:
            // A connection arrived, but it is closed
            // while waiting on the listen queue
            case ECONNABORTED:
            // just ignore interrupt
            case EINTR:
            // not an accept error, but protocal error
            case EPROTO:
            // Firewall rules forbid connection
            case EPERM:
            // The per-process limit on the number of
            // open file descriptors has been reached
            case EMFILE:
                errno = err_info;
                break;

            //* cases when the unexpected fatal errors occur

            // The system-wide limit on the total number of
            // open files has been reached
            case ENFILE:
            // The addr argument is not in a writable part of
            // the user address space
            case EFAULT:
                LOG_FATAL << "unexpected error occur when ::accept" << err_info;
                break;

            //* other unkown errors

            default:
                LOG_FATAL << "unkown error occur when ::accept" << err_info;
                break;
        }
    } else {
        setNonBlockAndCloseOnExec(connfd);
    }
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

// the wrapper of getsockname(2)
struct sockaddr_in getSockAddr(int socketfd) {
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof local_addr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof local_addr);
    if (::getsockname(socketfd, sockaddr_cast(&local_addr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getSockAddr";
    }
    return local_addr;
}

// the wrapper of getpeername(2)
struct sockaddr_in getPeerAddr(int socketfd) {
    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof peer_addr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peer_addr);
    if (::getpeername(socketfd, sockaddr_cast(&peer_addr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getPeerAddr";
    }
    return peer_addr;
}

} // namespace sockets
} // namespace net
} // namespace netlibcc