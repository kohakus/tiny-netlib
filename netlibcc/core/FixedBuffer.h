#ifndef NETLIBCC_CORE_FIXEDBUFFER_H_
#define NETLIBCC_CORE_FIXEDBUFFER_H_

#include <cstring>
#include <cassert>
#include <string>
#include "netlibcc/core/Noncopyable.h"

namespace netlibcc {
namespace detail {

const int kSmallBuffer = 4000;
// Large Buffer is about 4M bytes
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : Noncopyable {
public:
    FixedBuffer() : cur_(data_) {
        setCookie(cookieStart);
    }

    ~FixedBuffer() {
        setCookie(cookieEnd);
    }

    // methods for modification
    void setCookie(void (*cookie)()) { cookie_ = cookie; }
    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof data_); }
    void add(size_t len) { cur_ += len; }

    void append(const char* buf, size_t len) {
        if (avail() > static_cast<int>(len)) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    // member acquire methods
    const char* data() const { return data_; }
    char* current() { return cur_; }
    int length() const { return static_cast<int>(cur_ - data_); }
    int size() const { return length(); }
    int avail() const { return static_cast<int>(end() - cur_); }

    // methods for conversion
    std::string toString() const { return std::string(data_, length()); }

    // for GDB debug use
    const char* debugString();

private:
    // buffer end point
    const char* end() const {
        return data_ + sizeof data_;
    }

    // cookie functions
    static void cookieStart();
    static void cookieEnd();

    // data members
    void (*cookie_)();
    char data_[SIZE];
    char* cur_;
};

} // namespace detail
} // namespace netlibcc

#endif // NETLIBCC_CORE_FIXEDBUFFER_H_