#include "netlibcc/core/ThisThread.h"
#include <sys/types.h>
#include <type_traits>

namespace netlibcc {
namespace thisthread {

__thread int         t_cachedTid = 0;
__thread char        t_tidStr[32];
__thread int         t_tidStrLen = 6;
__thread const char* t_threadName = "unkown";

// use static_assert declaration to check the value type of pid_t
static_assert(std::is_same<int, pid_t>::value, "pid_t should appear as an int!");

} // namespace thisread
} // namespace netlibcc