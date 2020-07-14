#include "netlibcc/core/ThreadPool.h"

#include <unistd.h>
#include <cstdio>

#include "netlibcc/core/CountDownLatch.h"
#include "netlibcc/core/ThisThread.h"
#include "netlibcc/core/Logger.h"

void printTid() {
    printf("tid=%d\n", netlibcc::thisthread::tid());
}

void printString(const std::string& str) {
    LOG_INFO << str;
    usleep(100*1000);
}

void test(int max_qsize) {
    LOG_WARN << "Test ThreadPool with max task queue size: " << max_qsize;
    netlibcc::ThreadPool pool("Main Thread Pool");
    pool.setMaxQueSize(max_qsize);
    pool.start(5);

    LOG_WARN << "Adding Tasks";
    pool.addTask(printTid);
    pool.addTask(printTid);
    for (int i = 0; i < 100; i++) {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.addTask(std::bind(printString, std::string(buf)));
    }
    LOG_WARN << "Done";

    netlibcc::CountDownLatch latch(1);
    pool.addTask(std::bind(&netlibcc::CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
}

int main() {
    test(1);
    test(5);
    test(10);
    test(50);
}