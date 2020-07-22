#ifndef NETLIBCC_BASE_SCOKOPS_H_
#define NETLIBCC_BASE_SCOKOPS_H_

#include <arpa/inet.h>

namespace netlibcc {
namespace net {
namespace sockets {

// set socket file flag by using fcntl
int setNonBlockAndCloseOnExec(int socketfd);

void bind(int socketfd, const struct sockaddr* addr);
void listen(int socketfd);
int accept(int socketfd, struct sockaddr* addr);
int accept(int socketfd, struct sockaddr_in* addr4);

void close(int socketfd);
void shutdownWR(int socketfd);
void shutdownRD(int socketfd);

} // namespace sockets
} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_SCOKOPS_H_