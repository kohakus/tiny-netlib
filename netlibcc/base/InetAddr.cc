#include "netlibcc/base/InetAddr.h"
#include "netlibcc/base/SockUtils.h"
#include "netlibcc/base/Endian.h"
#include "netlibcc/core/Logger.h"

// check sockaddr_in struct
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");

namespace netlibcc {
namespace net {

InetAddr::InetAddr(StringArg ip_str, uint16_t port) {
    memset(&addr_, 0, sizeof addr_);
    if (sockets::fromIpPort(ip_str.c_str(), port, &addr_) <= 0) {
        LOG_SYSERR << "sockets::fromIpPort in InetAddr(StringArg, uint16_t)";
    }
}

InetAddr::InetAddr(uint16_t port, bool loopback) {
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = sockets::hostToNet32(loopback ? INADDR_LOOPBACK : INADDR_ANY);
    addr_.sin_port = sockets::hostToNet16(port);
}

uint16_t InetAddr::toPort() const {
    return sockets::netToHost16(portNetEndian());
}

std::string InetAddr::toIpStr() const {
    char buf[64] = "";
    sockets::toIpStr(buf, sizeof buf, &addr_);
    return buf;
}

std::string InetAddr::toIpPortStr() const {
    char buf[64] = "";
    sockets::toIpPortStr(buf, sizeof buf, &addr_);
    return buf;
}

const struct sockaddr* InetAddr::getSockAddr() const {
    return sockets::sockaddr_cast(&addr_);
}

const struct sockaddr_in* InetAddr::getSockAddrInet() {
    return &addr_;
}

void InetAddr::setSockAddrInet(const struct sockaddr_in& addr4) {
    addr_ = addr4;
}

} // namespace net
} // namespace netlibcc