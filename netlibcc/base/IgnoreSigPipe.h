#ifndef NETLIBCC_BASE_IGNORESIGPIPE_H_
#define NETLIBCC_BASE_IGNORESIGPIPE_H_

#include <signal.h>

namespace netlibcc {
namespace net {
namespace detail {

class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

} // namespace detail
} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_IGNORESIGPIPE_H_