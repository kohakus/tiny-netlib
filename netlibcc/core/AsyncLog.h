#ifndef NETLIBCC_CORE_ASYNCLOG_H_
#define NETLIBCC_CORE_ASYNCLOG_H_

#include <vector>
#include "netlibcc/core/Thread.h"
#include "netlibcc/core/LogStream.h"
#include "netlibcc/core/CountDownLatch.h"

namespace netlibcc {

class AsyncLog : Noncopyable {
public:
    AsyncLog(const std::string& basename, off_t rollsize, int flushgap=3);
    ~AsyncLog() {
        if (running_) {
            stop();
        }
    }

    // used by log frontends, appending log lines to current_buf_
    void append(const char*, int);

    void start() {
        running_ = true;
        thread_.start();
        // wait for log backend thread initialization to complete
        latch_.wait();
    }

    void stop() {
        running_ = false;
        cond_.notify();
        thread_.join();
    }

private:
    // the function used by log backend, which running in asynclog thread
    void threadFunc();

    // Large Buffer with 4M bytes
    using Buffer = detail::FixedBuffer<detail::kLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    // Vector with multiple buffers
    using BufferVec = std::vector<BufferPtr>;

    std::atomic<bool> running_;
    const off_t rollsize_;
    const int flushgap_;
    const std::string basename_;

    Thread         thread_;
    CountDownLatch latch_;
    Mutex          mutex_;
    Condition      cond_;

    // the current buffer that is used by log frontends
    BufferPtr current_buf_;
    // the spare buffer will be used if the current buffer is full
    BufferPtr next_buf_;
    // the full buffers that are going to be written to the log file, used by backend
    BufferVec buffers_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_ASYNCLOG_H_