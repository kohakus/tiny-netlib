#ifndef NETLIBCC_CORE_LOGSTREAM_H_
#define NETLIBCC_CORE_LOGSTREAM_H_

#include "netlibcc/core/FixedBuffer.h"
#include <fmt/format.h>

namespace netlibcc {

class LogStream : Noncopyable {
public:
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

    // deal with plain type value cases with format
    template<typename Plain>
    LogStream& operator<<(Plain v) {
        if (buffer_.avail() >= kMaxNumericSize) {
            const char* cptr = fmt::format_to(buffer_.current(), "{}", v);
            buffer_.add(cptr - buffer_.current());
        }
        return *this;
    }

    // some manually defined cases
    LogStream& operator<<(const void*);
    LogStream& operator<<(void*);
    LogStream& operator<<(double);
    LogStream& operator<<(float);
    LogStream& operator<<(const Buffer&);

    LogStream& operator<<(const char* str) {
        if (str) {
            buffer_.append(str, strlen(str));
        } else {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    LogStream& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(bool v) {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(const std::string& v) {
        buffer_.append(v.c_str(), v.length());
        return *this;
    }

    LogStream& operator<<(std::string&& v) {
        buffer_.append(v.c_str(), v.length());
        return *this;
    }

    // control and access functions
    void append(const char* data, int len) {
        buffer_.append(data, len);
    }

    void resetBuffer() { buffer_.reset(); }

    const Buffer& buffer() const { return buffer_; }

private:
    // short string buffer
    Buffer buffer_;
    static const int kMaxNumericSize = 32;
};

// Format quantity n in SI units (k, M, G, T, P, E)
std::string formatSI(int64_t n);

// Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
std::string formatIEC(int64_t n);

} // namespace netlibcc

#endif // NETLIBCC_CORE_LOGSTREAM_H_