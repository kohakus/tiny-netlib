#ifndef NETLIBCC_NET_TIMERID_H_
#define NETLIBCC_NET_TIMERID_H_

namespace netlibcc {
namespace net {

// forward declaration of Timer
class Timer;

// the interface class as Timer identity
class TimerId {
public:
    TimerId() : timer_(nullptr), seqid_(0) {}
    TimerId(Timer* timer, int64_t seqid) : timer_(timer), seqid_(seqid) {}
    ~TimerId() = default;

    friend class TimerContainer;

private:
    Timer* timer_;
    int64_t seqid_;
};

} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_NET_TIMERID_H_