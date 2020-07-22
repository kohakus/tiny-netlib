#include "netlibcc/base/SockUtils.h"
#include <cstring>
#include <cstdio>
#include <cassert>
#include "netlibcc/base/Endian.h"

namespace netlibcc {
namespace net {
namespace sockets {

// from ip and port to sockaddr_in struct

int fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr4) {
    addr4->sin_family = AF_INET;
    addr4->sin_port = hostToNet16(port);
    // from text to binary ipv4 address
    return ::inet_pton(AF_INET, ip, &addr4->sin_addr);
}

// from sockaddr to host byte represented str

void toIpStr(char* buf, size_t size, const struct sockaddr* addr) {
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
}

void toIpPortStr(char* buf, size_t size, const struct sockaddr* addr) {
    toIpStr(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = netToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}

void toIpStr(char* buf, size_t size, const struct sockaddr_in* addr4) {
    assert(addr4->sin_family == AF_INET);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
}

void toIpPortStr(char* buf, size_t size, const struct sockaddr_in* addr4) {
    assert(addr4->sin_family == AF_INET);
    toIpStr(buf, size, addr4);
    size_t end = ::strlen(buf);
    uint16_t port = netToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}

// socket address conversions

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr4) {
    return reinterpret_cast<const struct sockaddr*>(addr4);
}

struct sockaddr* sockaddr_cast(struct sockaddr_in* addr4) {
    return reinterpret_cast<struct sockaddr*>(addr4);
}

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) {
    return reinterpret_cast<const struct sockaddr_in*>(addr);
}

} // namespace sockets
} // namespace net
} // namespace netlibcc