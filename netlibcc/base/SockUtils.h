#ifndef NETLIBCC_BASE_SOCKUTILS_H_
#define NETLIBCC_BASE_SOCKUTILS_H_

#include <arpa/inet.h>

namespace netlibcc {
namespace net {
namespace sockets {

// from ip and port to sockaddr_in struct
int fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr4);

// from sockaddr to host byte represented str
void toIpStr(char* buf, size_t size, const struct sockaddr* addr);
void toIpPortStr(char* buf, size_t size, const struct sockaddr* addr);
void toIpStr(char* buf, size_t size, const struct sockaddr_in* addr4);
void toIpPortStr(char* buf, size_t size, const struct sockaddr_in* addr4);

// socket address conversions
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr4);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr4);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

} // namespace sockets
} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_SOCKUTILS_H_