#include "netlibcc/core/TimeAnchor.h"
#include <sys/time.h>
#include <cstdio>
#include <cinttypes>

namespace netlibcc {

std::string TimeAnchor::toStr() const {
    char buf[32];
    int64_t seconds = stamp_ / kMicroSecondsPerSecond;
    int64_t residual_micro = stamp_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, residual_micro);
    return buf;
}

std::string TimeAnchor::formatMicro() const {
    char buf[64];
    time_t seconds = static_cast<time_t>(stamp_ / kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(stamp_ % kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    return buf;
}

std::string TimeAnchor::formatSeconds() const {
    char buf[64];
    time_t seconds = static_cast<time_t>(stamp_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    return buf;
}

TimeAnchor TimeAnchor::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return TimeAnchor(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

} // namespace netlibcc