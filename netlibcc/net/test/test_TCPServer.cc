#include "netlibcc/net/TCPServer.h"
#include "netlibcc/net/EventLoop.h"
#include "netlibcc/base/InetAddr.h"
#include "netlibcc/core/Thread.h"
#include "netlibcc/core/Logger.h"

#include <unistd.h>
#include <utility>
#include <cstdio>

using namespace netlibcc;
using namespace netlibcc::net;
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;

int numThreads = 0;

class EchoServer {
public:
    EchoServer(EventLoop* loop, const InetAddr& listen_addr)
        : loop_(loop), server_(loop_, listen_addr, "EchoServer") {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2));
        server_.setThreadNum(numThreads);
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TCPConnectionPtr& conn) {
        LOG_TRACE << conn->sockAddr().toIpPortStr() << " -> "
                  << conn->peerAddr().toIpPortStr() << " is "
                  << (conn->isConnected() ? "connected" : "down");
        conn->send("hello\n");
    }

    void onMessage(const TCPConnectionPtr& conn, Buffer* buf) {
        string msg(buf->retrieveAllAsStr());
        LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes";
        if (msg == "exit\n") {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n") {
            loop_->quit();
        }
        conn->send(msg);
    }

    EventLoop* loop_;
    TCPServer server_;
};

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << getpid() << ", tid = " << thisthread::tid();
    LOG_INFO << "sizeof TCPConnection = " << sizeof(TCPConnection);
    if (argc > 1) {
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    InetAddr listen_addr(9900, false);
    EchoServer server(&loop, listen_addr);

    server.start();
    loop.loop();
}