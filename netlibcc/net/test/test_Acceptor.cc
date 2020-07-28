#include "netlibcc/net/Acceptor.h"

#include <unistd.h>
#include <cstdio>

#include "netlibcc/net/EventLoop.h"
#include "netlibcc/base/SockOps.h"
#include "netlibcc/base/InetAddr.h"

using namespace netlibcc;
using namespace netlibcc::net;

void newConnection(int sockfd, const InetAddr& peer_addr) {
    printf("accept a new connection from %s\n", peer_addr.toIpStr().c_str());
    ::write(sockfd, "Hello, how are you?\n", 20);
    sockets::close(sockfd);
}

int main() {
    printf("main() pid = %d\n", getpid());

    InetAddr listen_addr(9900);
    EventLoop loop;

    Acceptor acceptor(&loop, listen_addr, false);
    acceptor.setNewConnCallback(newConnection);
    acceptor.listen();

    loop.loop();
}