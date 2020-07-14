#ifndef NETLIBCC_CORE_THISTHREAD_H_
#define NETLIBCC_CORE_THISTHREAD_H_

namespace netlibcc {
namespace thisthread {

// TLS data (Thread Local Storage)
extern __thread int         t_cachedTid;
extern __thread char        t_tidStr[32];
extern __thread int         t_tidStrLen;
extern __thread const char* t_threadName;

// the tid of a thread should be cached after it's acquisition.
void cacheTid();

inline int tid() {
    // only need to be cached on the first call
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidStr() {
    return t_tidStr;
}

inline const char* name() {
    return t_threadName;
}

inline int tidStrLen() {
    return t_tidStrLen;
}

} // namespace thisthred
} // namespace netlibcc

#endif // NETLIBCC_CORE_THISTHREAD_H_