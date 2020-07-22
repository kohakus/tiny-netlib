#include "netlibcc/net/EventLoop.h"
#include "netlibcc/core/Thread.h"

#include <unistd.h>
#include <cassert>
#include <cstdio>

using namespace netlibcc;
using namespace netlibcc::net;

EventLoop* g_loop;

void callback() {
    printf("callback(): pid = %d, tid = %d\n", getpid(), thisthread::tid());

    // Negative test: the creation of a new EventLoop obj will be aborted.
    // Since in this thread, we have an EventLoop obj already.
    EventLoop another_loop;
}

void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), thisthread::tid());

    assert(EventLoop::getCurrentThreadLoop() == nullptr);
    EventLoop loop;
    assert(EventLoop::getCurrentThreadLoop() == &loop);

    loop.runAfter(2.0, callback);
    loop.loop();
}

int main() {
    printf("main(): pid = %d, tid = %d\n", getpid(), thisthread::tid());

    assert(EventLoop::getCurrentThreadLoop() == nullptr);
    EventLoop loop;
    assert(EventLoop::getCurrentThreadLoop() == &loop);

    Thread thread(threadFunc);
    thread.start();

    loop.loop();
}