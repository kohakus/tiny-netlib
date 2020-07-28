#include "netlibcc/net/IOThreadPool.h"

#include <unistd.h>
#include <cstdio>

#include "netlibcc/net/EventLoop.h"
#include "netlibcc/core/Thread.h"

using namespace netlibcc;
using namespace netlibcc::net;

void showId(EventLoop* loop = nullptr) {
    printf("showId: pid = %d, tid = %d, loop = %p\n", getpid(), thisthread::tid(), loop);
}

void threadFunc(EventLoop* loop) {
    printf("threadFunc: pid = %d, tid = %d, loop = %p\n", getpid(), thisthread::tid(), loop);
}

int main() {
    showId();

    EventLoop loop;
    loop.runAfter(11, std::bind(&EventLoop::quit, &loop));

    // only main thread
    {
        printf("Single thread %p:\n", &loop);
        IOThreadPool pool(&loop, "single");
        pool.setThreadNum(0);
        pool.start(threadFunc);
        assert(pool.getNextLoop() == &loop);
        assert(pool.getNextLoop() == &loop);
        assert(pool.getNextLoop() == &loop);
    }

    // main thread + another IOThread
    {
        printf("Another Thread:\n");
        IOThreadPool pool(&loop, "another");
        pool.setThreadNum(1);
        pool.start(threadFunc);
        EventLoop* next_loop = pool.getNextLoop();
        next_loop->runAfter(2, std::bind(showId, next_loop));
        assert(next_loop != &loop);
        assert(next_loop == pool.getNextLoop());
        assert(next_loop == pool.getNextLoop());
        ::sleep(3);
    }

    // main thread + 3 other threads
    {
        printf("Three other threads:\n");
        IOThreadPool pool(&loop, "three");
        pool.setThreadNum(3);
        pool.start(threadFunc);
        EventLoop* next_loop = pool.getNextLoop();
        next_loop->runInLoop(std::bind(showId, next_loop));
        assert(next_loop != &loop);
        assert(next_loop != pool.getNextLoop());
        assert(next_loop != pool.getNextLoop());
        assert(next_loop == pool.getNextLoop());
    }

    loop.loop();
}