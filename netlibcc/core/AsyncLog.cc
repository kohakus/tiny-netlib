#include "netlibcc/core/AsyncLog.h"
#include "netlibcc/core/LogFile.h"
#include "netlibcc/core/TimeAnchor.h"

namespace netlibcc {

AsyncLog::AsyncLog(const std::string& basename, off_t rollsize, int flushgap)
            : running_(false),
              rollsize_(rollsize),
              flushgap_(flushgap),
              basename_(basename),
              thread_(std::bind(&AsyncLog::threadFunc, this), "AsyncLogging"),
              latch_(1),
              mutex_(),
              cond_(mutex_),
              current_buf_(new Buffer),
              next_buf_(new Buffer),
              buffers_() {
    current_buf_->bzero();
    next_buf_->bzero();
    buffers_.reserve(16);
}

// frontend method, used by multiple threads
void AsyncLog::append(const char* str_line, int len) {
    MutexLockGuard lock(mutex_);
    // double-buffer replacement strategy
    if (current_buf_->avail() > len) {
        current_buf_->append(str_line, len);
    } else {
        buffers_.push_back(std::move(current_buf_));
        // unique_ptr that is moved from is set to nullptr
        if (next_buf_) {
            // transfer ownership
            current_buf_ = std::move(next_buf_);
        } else {
            // the number of frontend buffer is not enough, we need more
            current_buf_.reset(new Buffer);
        }
        current_buf_->append(str_line, len);
        // notify backend thread
        cond_.notify();
    }
}

void AsyncLog::threadFunc() {
    assert(running_ = true);
    // notify main thread
    latch_.countDown();
    // here we use unlocked LogFile
    LogFile output(basename_, rollsize_, false);

    // backend also has two buffers, used for swapping
    BufferPtr swap_currbuf(new Buffer);
    BufferPtr swap_nextbuf(new Buffer);
    BufferVec swap_buffers;
    swap_currbuf->bzero();
    swap_nextbuf->bzero();
    swap_buffers.reserve(16);

    while (running_) {
        assert(swap_currbuf && swap_currbuf->length() == 0);
        assert(swap_nextbuf && swap_nextbuf->length() == 0);
        assert(swap_buffers.empty());

        // swap strategy
        {
            MutexLockGuard lock(mutex_);
            // wait for full buffers
            if (buffers_.empty()) {
                cond_.waitSecs(rollsize_);
            }

            buffers_.push_back(std::move(current_buf_));
            current_buf_ = std::move(swap_currbuf);
            swap_buffers.swap(buffers_);

            if (!next_buf_) {
                next_buf_ = std::move(swap_nextbuf);
            }
        }

        // deal with overmuch accumulated buffers
        // there may be something wrong in the log frontend
        if (swap_buffers.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof buf, "Overmuch accumulated buffers! Dropped log message at %s.\n",
                    TimeAnchor::now().formatMicro().c_str());
            output.append(buf, static_cast<int>(strlen(buf)));
            // drop buffers
            swap_buffers.erase(swap_buffers.begin()+2, swap_buffers.end());
        }

        // now we can deal with file writing without locking
        for (const auto& buffer : swap_buffers) {
            output.append(buffer->data(), buffer->length());
        }

        // avoid trashing
        if (swap_buffers.size() > 2) { swap_buffers.resize(2); }

        if (!swap_currbuf) {
            assert(!swap_buffers.empty());
            swap_currbuf = std::move(swap_buffers.back());
            swap_buffers.pop_back();
            swap_currbuf->reset();
        }

        if (!swap_nextbuf) {
            assert(!swap_buffers.empty());
            swap_nextbuf = std::move(swap_buffers.back());
            swap_buffers.pop_back();
            swap_nextbuf->reset();
        }

        swap_buffers.clear();
        output.flush();
    }
    output.flush();
}

} // namespace netlibcc