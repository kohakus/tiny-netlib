#ifndef NETLIBCC_BASE_BUFFER_H_
#define NETLIBCC_BASE_BUFFER_H_

#include <algorithm>
#include <vector>
#include <cassert>

#include "netlibcc/core/StringArg.h"

namespace netlibcc {
namespace net {

class Buffer {
public:
    static const size_t kPrepend = 8;
    static const size_t kInitSize = 2040;

    // ctor
    explicit Buffer(size_t init_size = kInitSize)
        : buffer_(kPrepend + init_size),
          ridx_(kPrepend),
          widx_(kPrepend) {
        assert(readableBytes()+writableBytes()+kPrepend == buffer_.size());
    }

    //* state functions

    size_t readableBytes() const {
        return widx_ - ridx_;
    }

    size_t writableBytes() const {
        return buffer_.size() - widx_;
    }

    size_t prependableBytes() const {
        return ridx_ - kPrepend;
    }

    size_t capacity() const {
        return buffer_.capacity();
    }

    //* pointer position access functions

    const char* rpos() const {
        return bufferBegin() + ridx_;
    }

    const char* wpos() const {
        return bufferBegin() + widx_;
    }

    char* wpos() {
        return bufferBegin() + widx_;
    }

    //* operations to move ridx_ forward

    void retrieve(size_t len);
    void retrieveUntil(const char* end);

    // all data has been retrieved
    void retrieveAll() {
        ridx_ = kPrepend;
        widx_ = kPrepend;
    }

    std::string retrieveAsStr(size_t len);

    std::string retrieveAllAsStr() {
        return retrieveAsStr(readableBytes());
    }

    //* operations to move widx_ forward

    void append(const std::string& str) {
        append(str.c_str(), str.size());
    }

    void append(const std::string&& str) {
        append(str.c_str(), str.size());
    }

    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    void append(const char* data, size_t len);

    // auxiliary function to prepend
    void prepend(const void* data, size_t len);

    // adjust space if need
    void checkAdjustSpace(size_t len) {
        if (writableBytes() < len) {
            adjustSpace(len);
        }
        assert(writableBytes() >= len);
    }

    // swap two Buffers
    void swap(Buffer& buf);

    // find CRLF for spliting lines
    const char* findCRLF(const char* start) const;
    const char* findCRLF() const;

    // Read data from file descriptor into buffer
    ssize_t readFromFd(int fd, int& err_info);

    // TODO: Support for block data with multiple bytes

private:
    void widxForward(size_t len) {
        assert(len <= writableBytes());
        widx_ += len;
    }

    void widxBackward(size_t len) {
        assert(len <= readableBytes());
        widx_ -= len;
    }

    // vector as a continuous space
    char* bufferBegin() { return &*buffer_.begin(); }
    const char* bufferBegin() const { return &*buffer_.begin(); }

    // adjust space to make sure residual writable space is enough
    void adjustSpace(size_t len);

    std::vector<char> buffer_;

    //* indicator of double pointers, which represents a
    // "Left closed and right open" range
    size_t ridx_;
    size_t widx_;

    // CRLF bytes
    static const char kCRLF[];
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_BUFFER_H_
