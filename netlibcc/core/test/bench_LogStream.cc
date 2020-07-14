#include "netlibcc/core/LogStream.h"
#include "netlibcc/core/TimeAnchor.h"

#include <cstdio>
#include <cinttypes>

using namespace netlibcc;

const size_t N = 1000000;

template<typename T>
void benchStandardIO(const char* fmtstr) {
    char buf[32];
    TimeAnchor start(TimeAnchor::now());
    for (size_t i = 0; i < N; i++) {
        snprintf(buf, sizeof buf, fmtstr, (T)(i));
    }
    TimeAnchor end(TimeAnchor::now());
    printf("Printf cost time: %f\n", timeDiff(end, start));
}

template<typename T>
void benchLogStream() {
    TimeAnchor start(TimeAnchor::now());
    LogStream os;
    for (size_t i = 0; i < N; i++) {
        os << (T)(i);
        os.resetBuffer();
    }
    TimeAnchor end(TimeAnchor::now());
    printf("LogStream cost time: %f\n", timeDiff(end, start));
}

int main() {
    puts("int");
    benchStandardIO<int>("%d");
    benchLogStream<int>();

    puts("double");
    benchStandardIO<double>("%.12g");
    benchLogStream<double>();

    puts("int64_t");
    benchStandardIO<int64_t>("%" PRId64);
    benchLogStream<int64_t>();

    puts("void*");
    benchStandardIO<void*>("%p");
    benchLogStream<void*>();
}