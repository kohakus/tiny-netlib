#include "netlibcc/net/EventLoop.h"
#include "netlibcc/net/IOThread.h"

#include <unistd.h>
#include <cstdio>

using namespace netlibcc;
using namespace netlibcc::net;

// global counter
int cnt = 0;

// global accessible event loop
EventLoop* g_loop;

void showTid() {
    printf("pid = %d, tid = %d\n", getpid(), thisthread::tid());
    printf("now %s\n", TimeAnchor::now().toStr().c_str());
}

void print(const char* msg) {
    printf("msg %s %s\n", TimeAnchor::now().toStr().c_str(), msg);
    if (++cnt == 5) {
        g_loop->quit();
    }
}

void cancelTimer(TimerId timerid) {
    g_loop->cancelTimer(timerid);
    printf("cancel at %s\n", TimeAnchor::now().toStr().c_str());
}

int main() {
    showTid();
    sleep(1);

    // test single thread eventloop
    {
        EventLoop loop;
        g_loop = &loop;
        print("main loop");

        // register timeout callbacks
        loop.runAfter(1.0, std::bind(print, "once1"));
        loop.runAfter(1.5, std::bind(print, "once1.5"));
        loop.runAfter(2.5, std::bind(print, "once2.5"));
        loop.runAfter(3.5, std::bind(print, "once3.5"));
        TimerId t1 = loop.runAfter(4.5, std::bind(print, "once4.5"));
        loop.runAfter(4.2, std::bind(cancelTimer, t1));
        loop.runAfter(4.8, std::bind(cancelTimer, t1));

        loop.loop();
        print("main loop exits");
    }

    sleep(1);
    // test timer container with IO thread
    {
        IOThread loop_thread;
        EventLoop* loop = loop_thread.start();
        loop->runAfter(2.0, showTid);
        sleep(3);
        print("thread loop exits");
    }
}