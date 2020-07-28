#include "netlibcc/base/Buffer.h"

// for scatter read and gather write
#include <sys/uio.h>
#include <errno.h>

namespace netlibcc {
namespace net {

const size_t Buffer::kPrepend;
const size_t Buffer::kInitSize;

const char Buffer::kCRLF[] = "\r\n";

void Buffer::retrieve(size_t len) {
    size_t readable_bytes = readableBytes();
    assert(len <= readable_bytes);
    if (len < readable_bytes) {
        ridx_ += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveUntil(const char* end) {
    // check if end pointer is valid
    assert(end >= rpos());
    assert(end <= wpos());
    retrieve(end - rpos());
}

std::string Buffer::retrieveAsStr(size_t len) {
    assert(len <= readableBytes());

    // the order is important
    std::string ans(rpos(), len);
    retrieve(len);

    return ans;
}

void Buffer::append(const char* data, size_t len) {
    checkAdjustSpace(len);
    std::copy(data, data+len, wpos());
    widxForward(len);
}

void Buffer::adjustSpace(size_t len) {
    size_t residual_size = writableBytes() + prependableBytes();
    if (residual_size < len) {
        size_t target_size = len - residual_size + buffer_.size();
        if (buffer_.capacity() < target_size) {
            // (1). in this case, the current buffer capacity is not enough,
            //      a new buffer space is necessary
            std::vector<char> new_buffer;
            size_t new_capacity = buffer_.capacity();
            while (new_capacity < target_size) {
                // use factor 2 to find a proper space size
                new_capacity += new_capacity;
            }
            new_buffer.reserve(new_capacity);
            new_buffer.resize(target_size);
            std::copy(bufferBegin(), bufferBegin()+kPrepend, new_buffer.begin());

            size_t readable_bytes = readableBytes();
            std::copy(bufferBegin() + ridx_,
                      bufferBegin() + widx_,
                      new_buffer.begin() + kPrepend);
            buffer_.swap(new_buffer);
            ridx_ = kPrepend;
            widx_ = ridx_ + readable_bytes;
            assert(readable_bytes == readableBytes());
        } else if (buffer_.capacity() < widx_ + len) {
            // (2). the current buffer capacity is enough, but the available
            //      right-hand appending capacity is not enough

            if (widx_ > buffer_.capacity()/2 && readableBytes() < buffer_.capacity()/2) {
                assert(kPrepend < ridx_);
                size_t readable_bytes = readableBytes();
                std::copy(bufferBegin() + ridx_,
                          bufferBegin() + widx_,
                          bufferBegin() + kPrepend);
                ridx_ = kPrepend;
                widx_ = ridx_ + readable_bytes;
                assert(readable_bytes == readableBytes());
                assert(widx_+len == target_size);
                buffer_.resize(widx_ + len);
            } else {
                buffer_.resize(widx_ + len);
            }

        } else {
            // (3). the available appending space is enough
            buffer_.resize(widx_ + len);
        }
    } else {
        // in this case, just move the current data to the front,
        // in this way the residual writable space is enough
        assert(kPrepend < ridx_);
        size_t readable_bytes = readableBytes();
        std::copy(bufferBegin() + ridx_,
                  bufferBegin() + widx_,
                  bufferBegin() + kPrepend);
        ridx_ = kPrepend;
        widx_ = ridx_ + readable_bytes;
        assert(readable_bytes == readableBytes());
    }
}

void Buffer::prepend(const void* data, size_t len) {
    assert(len <= prependableBytes());
    ridx_ -= len;
    const char* data_begin = static_cast<const char*>(data);
    std::copy(data_begin, data_begin+len, bufferBegin()+ridx_);
}

void Buffer::swap(Buffer& buf) {
    buffer_.swap(buf.buffer_);
    std::swap(ridx_, buf.ridx_);
    std::swap(widx_, buf.widx_);
}

const char* Buffer::findCRLF() const {
    return findCRLF(rpos());
}

const char* Buffer::findCRLF(const char* start) const {
    assert(start >= rpos());
    assert(start <= wpos());

    // use std::search to find a sub-sequence O(S*N)
    // FIXME: more efficient implementation
    const char* crlf_pos = std::search(start, wpos(), kCRLF, kCRLF+2);
    return crlf_pos == wpos() ? nullptr : crlf_pos;
}


// Use scatter read to save a ioctl()+FIONREAD system call
// ***************** (From Linux Manual Page) ****************
//
//    ssize_t readv(int fd, const struct iovec* iov, int iovcnt);
//
//    struct iovec {
//        void*  iov_base;    /* Starting address */
//        size_t iov_len;     /* Number of bytes to transfer */
//    };
//
// ***************** (From Linux Manual Page) ****************
ssize_t Buffer::readFromFd(int fd, int& err_info) {
    // space on stack
    char extra_buffer[65536];

    // two targets, buffer_ and space on stack
    struct iovec vec[2];

    const size_t writable_bytes = writableBytes();

    // space on buffer_
    vec[0].iov_base = bufferBegin() + widx_;
    vec[0].iov_len = writable_bytes;

    // space on stack
    vec[1].iov_base = extra_buffer;
    vec[1].iov_len = sizeof extra_buffer;

    const ssize_t n = ::readv(fd, vec,
                              (writable_bytes < sizeof extra_buffer) ? 2 : 1);
    if (n < 0) {
        // error occurred
        err_info = errno;
    } else if (writable_bytes >= static_cast<size_t>(n)){
        // buffer_ residual writable space is enough
        widx_ += n;
    } else {
        widx_ = buffer_.size();
        append(extra_buffer, n - writable_bytes);
    }
    return n;
}

} // namespace net
} // namespace netlibcc