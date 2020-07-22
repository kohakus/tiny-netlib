#ifndef NETLIBCC_BASE_SOCKET_H_
#define NETLIBCC_BASE_SOCKET_H_

#include "netlibcc/core/Noncopyable.h"

// forward declaration of tcp_info struct, which is included in <netinet/tcp.h>
// https://linuxgazette.net/136/pfeiffer.html
// (put it here for the time being, it is not needed yet)
// struct tcp_info;

namespace netlibcc {
namespace net {

// forward declaration of InetAddr
class InetAddr;

// RAII class for socket fd
class Socket : Noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    // socket operations
    void listen();
    void bind(const InetAddr& local_addr);
    int accept(InetAddr* peer_addr);
    void shutdownWrite();

    // access function
    int fd() const {
        return sockfd_;
    }

    // Option: Enable/Disable Nagle's algorithm
    bool setTcpNoDelay(bool on);

    // Option: Enable/Disable SO_REUSEADDR
    bool setReuseAddr(bool on);

    // Option: Enable/Disable SO_REUSEPORT
    bool setReusePort(bool on);

    // Option: Enable/Disable SO_KEEPALIVE
    bool setKeepAlive(bool on);

private:
    const int sockfd_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_SOCKET_H_