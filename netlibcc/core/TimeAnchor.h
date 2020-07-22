#ifndef NETLIBCC_CORE_TIMEANCHOR_H_
#define NETLIBCC_CORE_TIMEANCHOR_H_

#include <string>

namespace netlibcc {

// simple UTC time stamp class
class TimeAnchor {
public:
    TimeAnchor() : stamp_(0) {}
    TimeAnchor(int64_t stamp) : stamp_(stamp) {}

    // get string of stamp_ value
    std::string toStr() const;
    // get format string with micro seconds
    std::string formatMicro() const;
    // get format string with seconds
    std::string formatSeconds() const;

    // get time stamp value
    int64_t timeMicro() const { return stamp_; }
    time_t timeSeconds() const {
        return static_cast<time_t>(stamp_ / kMicroSecondsPerSecond);
    }

    // get time stamp that represents the time of now
    static TimeAnchor now();

    // weight from seconds to micro seconds
    static const int kMicroSecondsPerSecond = 1000000;

private:
    // micro seconds since epoch
    int64_t stamp_;
};

inline bool operator<(TimeAnchor lhs, TimeAnchor rhs) {
    return lhs.timeMicro() < rhs.timeMicro();
}

inline bool operator==(TimeAnchor lhs, TimeAnchor rhs) {
    return lhs.timeMicro() == rhs.timeMicro();
}

inline bool operator<=(TimeAnchor lhs, TimeAnchor rhs) {
    return lhs == rhs || lhs < rhs;
}

inline TimeAnchor operator-(TimeAnchor lhs, TimeAnchor rhs) {
    return TimeAnchor(lhs.timeMicro() - rhs.timeMicro());
}

inline double timeDiff(TimeAnchor high, TimeAnchor low) {
    TimeAnchor diff = high - low;
    return static_cast<double>(diff.timeMicro()) / TimeAnchor::kMicroSecondsPerSecond;
}

} // namespace netlibcc

#endif // NETLIBCC_CORE_TIMEANCHOR_H_