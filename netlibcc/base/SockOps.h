#ifndef NETLIBCC_BASE_SCOKOPS_H_
#define NETLIBCC_BASE_SCOKOPS_H_

#include <arpa/inet.h>

namespace netlibcc {
namespace net {
namespace sockets {

// set socket file flag by using fcntl
int setNonBlockAndCloseOnExec(int socketfd);

//* basic socket operations
int create(sa_family_t family);
void bind(int socketfd, const struct sockaddr* addr);
void listen(int socketfd);
int accept(int socketfd, struct sockaddr* addr);
int accept(int socketfd, struct sockaddr_in* addr4);

//* close and half-close
void close(int socketfd);
void shutdownWR(int socketfd);
void shutdownRD(int socketfd);

//* get addresses from socket fd
struct sockaddr_in getSockAddr(int socketfd);
struct sockaddr_in getPeerAddr(int socketfd);

} // namespace sockets
} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_SCOKOPS_H_