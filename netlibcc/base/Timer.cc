#include "netlibcc/base/Timer.h"

namespace netlibcc {
namespace net {

std::atomic_int64_t Timer::numCreated_;

Timer::Timer(std::function<void()> callback, TimeAnchor expiration, double interval)
        : callback_(callback),
          expiration_(expiration),
          interval_(interval),
          seqid_(++numCreated_),
          timer_pos_(-1) {}

void Timer::setPosInHeap(size_t heap_pos) {
    // FIXME: use more proper type to avoid static casting
    timer_pos_ = static_cast<int>(heap_pos);
}

void Timer::setPosInHeap(const int heap_pos) {
    timer_pos_ = heap_pos;
}

} // namespace net
} // namespace netlibcc