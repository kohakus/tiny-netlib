#include "netlibcc/core/LogStream.h"
#include <cstdint>

namespace netlibcc {

LogStream& LogStream::operator<<(const Buffer& v) {
    int vlen = v.length();
    if (buffer_.avail() > vlen) {
        memcpy(buffer_.current(), v.data(), vlen);
        buffer_.add(vlen);
    }
    return *this;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.avail() >= kMaxNumericSize) {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        const char* cptr = fmt::format_to(buf+2, "{:X}", v);
        buffer_.add(cptr - buf);
    }
    return *this;
}

LogStream& LogStream::operator<<(void* p) {
    return operator<<(reinterpret_cast<const void*>(p));
}

LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        const char* cptr = fmt::format_to(buffer_.current(), "{:.12g}", v);
        buffer_.add(cptr - buffer_.current());
    }
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    return operator<<(static_cast<double>(v));
}

// Format a number with 5 characters, including SI units.
// [0,     999]
// [1.00k, 999k]
// [1.00M, 999M]
// [1.00G, 999G]
// [1.00T, 999T]
// [1.00P, 999P]
// [1.00E, inf)
std::string formatSI(int64_t s) {
    double n = static_cast<double>(s);
    if (s < 1000) {
        return fmt::format("{:d}", s);
    } else if (s < 9995) {
        return fmt::format("{:.2f}k", n / 1e3);
    } else if (s < 99950) {
        return fmt::format("{:.1f}k", n / 1e3);
    } else if (s < 999500) {
        return fmt::format("{:.0f}k", n / 1e3);
    } else if (s < 9995000) {
        return fmt::format("{:.2f}M", n / 1e6);
    } else if (s < 99950000) {
        return fmt::format("{:.1f}M", n / 1e6);
    } else if (s < 999500000) {
        return fmt::format("{:.0f}M", n / 1e6);
    } else if (s < 9995000000) {
        return fmt::format("{:.2f}G", n / 1e9);
    } else if (s < 99950000000) {
        return fmt::format("{:.1f}G", n / 1e9);
    } else if (s < 999500000000) {
        return fmt::format("{:.0f}G", n / 1e9);
    } else if (s < 9995000000000) {
        return fmt::format("{:.2f}T", n / 1e12);
    } else if (s < 99950000000000) {
        return fmt::format("{:.1f}T", n / 1e12);
    } else if (s < 999500000000000) {
        return fmt::format("{:.0f}T", n / 1e12);
    } else if (s < 9995000000000000) {
        return fmt::format("{:.2f}P", n / 1e15);
    } else if (s < 99950000000000000) {
        return fmt::format("{:.1f}P", n / 1e15);
    } else if (s < 999500000000000000) {
        return fmt::format("{:.0f}P", n / 1e15);
    } else {
        return fmt::format("{:.2f}E", n / 1e18);
    }
}


// [0, 1023]
// [1.00Ki, 9.99Ki]
// [10.0Ki, 99.9Ki]
// [ 100Ki, 1023Ki]
// [1.00Mi, 9.99Mi]
std::string formatIEC(int64_t s) {
    double n = static_cast<double>(s);
    const double Ki = 1024.0;
    const double Mi = Ki * 1024.0;
    const double Gi = Mi * 1024.0;
    const double Ti = Gi * 1024.0;
    const double Pi = Ti * 1024.0;
    const double Ei = Pi * 1024.0;

    if (n < Ki) {
        return fmt::format("%d", s);
    } else if (n < Ki*9.995) {
        return fmt::format("{:.2f}Ki", n / Ki);
    } else if (n < Ki*99.95) {
        return fmt::format("{:.1f}Ki", n / Ki);
    } else if (n < Ki*1023.5) {
        return fmt::format("{:.0f}Ki", n / Ki);
    } else if (n < Mi*9.995) {
        return fmt::format("{:.2f}Mi", n / Mi);
    } else if (n < Mi*99.95) {
        return fmt::format("{:.1f}Mi", n / Mi);
    } else if (n < Mi*1023.5) {
        return fmt::format("{:.0f}Mi", n / Mi);
    } else if (n < Gi*9.995) {
        return fmt::format("{:.2f}Gi", n / Gi);
    } else if (n < Gi*99.95) {
        return fmt::format("{:.1f}Gi", n / Gi);
    } else if (n < Gi*1023.5) {
        return fmt::format("{:.0f}Gi", n / Gi);
    } else if (n < Ti*9.995) {
        return fmt::format("{:.2f}Ti", n / Ti);
    } else if (n < Ti*99.95) {
        return fmt::format("{:.1f}Ti", n / Ti);
    } else if (n < Ti*1023.5) {
        return fmt::format("{:.0f}Ti", n / Ti);
    } else if (n < Pi*9.995) {
        return fmt::format("{:.2f}Pi", n / Pi);
    } else if (n < Pi*99.95) {
        return fmt::format("{:.1f}Pi", n / Pi);
    } else if (n < Pi*1023.5) {
        return fmt::format("{:.0f}Pi", n / Pi);
    } else if (n < Ei*9.995) {
        return fmt::format("{:.2f}Ei", n / Ei);
    } else {
        return fmt::format("{:.1f}Ei", n / Ei);
    }
}

} // namespace netlibcc