#ifndef NETLIBCC_BASE_INETADDR_H_
#define NETLIBCC_BASE_INETADDR_H_

// Internet address family
// https://pubs.opengroup.org/onlinepubs/000095399/basedefs/netinet/in.h.html
#include <netinet/in.h>

#include <string>
#include "netlibcc/core/StringArg.h"

//    /* Structure describing an Internet socket address. */
//    struct sockaddr_in {
//        sa_family_t    sin_family; /* address family: AF_INET */
//        uint16_t       sin_port;   /* port in network byte order */
//        struct in_addr sin_addr;   /* internet address */
//    };

//    /* Internet address. */
//    typedef uint32_t in_addr_t;
//    struct in_addr {
//        in_addr_t       s_addr;     /* address in network byte order */
//    };

//    struct sockaddr_in6 {
//        sa_family_t     sin6_family;   /* address family: AF_INET6 */
//        uint16_t        sin6_port;     /* port in network byte order */
//        uint32_t        sin6_flowinfo; /* IPv6 flow information */
//        struct in6_addr sin6_addr;     /* IPv6 address */
//        uint32_t        sin6_scope_id; /* IPv6 scope-id */
//    };

namespace netlibcc {
namespace net {

// class of IPv4 socket address
class InetAddr {
public:
    // in this case we want to construct an endpoint directly
    // from a given port and ip address "X.X.X.X"
    InetAddr(StringArg ip_str, uint16_t port);

    // in this case we may want to acquire address, often in listen phase
    explicit InetAddr(uint16_t port = 0, bool loopback = false);

    // in this case we want to construct an endpoint directly
    // from sockaddr_in struct, often in accept phase
    explicit InetAddr(const struct sockaddr_in& addr) : addr_(addr) {}

    ~InetAddr() = default;

    // to host byte order
    uint16_t toPort() const;
    std::string toIpStr() const;
    std::string toIpPortStr() const;

    // access functions

    sa_family_t family() const {
        return addr_.sin_family;
    }

    uint32_t ipNetEndian() const {
        return addr_.sin_addr.s_addr;
    }

    uint16_t portNetEndian() const {
        return addr_.sin_port;
    }

    const struct sockaddr* getSockAddr() const;
    const struct sockaddr_in* getSockAddrInet();
    void setSockAddrInet(const struct sockaddr_in&);

private:
    // IPv4 address field (AF_INET)
    struct sockaddr_in addr_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_INETADDR_H_