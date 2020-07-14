#include "netlibcc/core/FixedBuffer.h"

namespace netlibcc {
namespace detail {

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template<int SIZE>
const char* FixedBuffer<SIZE>::debugString() {
    *cur_ = '\0';
    return data_;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart() {}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd() {}

} // namespace detail
} // namespace netlibcc