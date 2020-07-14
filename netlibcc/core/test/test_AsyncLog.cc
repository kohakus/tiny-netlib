#include <sys/resource.h>
#include <unistd.h>
#include <cstdio>

#include "netlibcc/core/AsyncLog.h"
#include "netlibcc/core/Logger.h"
#include "netlibcc/core/TimeAnchor.h"

using namespace netlibcc;

off_t kRollSize = 500 * 1000 * 1000;

AsyncLog* g_asynclog = nullptr;

// for Logger output function registration
void asyncOutput(const char* msg, int len) {
    g_asynclog->append(msg, len);
}

void bench(bool long_log) {
    Logger::setOutput(asyncOutput);

    int cnt = 0;
    const int kBatch = 1000;
    std::string empty = " ";
    std::string long_str(3000, 'X');
    long_str += " ";

    for (int t = 0; t < 30; t++) {
        TimeAnchor start = TimeAnchor::now();
        for (int i = 0; i < kBatch; i++) {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                     << (long_log ? long_str : empty) << cnt;
            cnt++;
        }
        TimeAnchor end = TimeAnchor::now();
        printf("%f\n", timeDiff(end, start)*1000000/kBatch);
        struct timespec ts = { 0, 500*1000*1000 };
        nanosleep(&ts, nullptr);
    }
}

int main(int argc, char* argv[]) {
    {
        size_t kOneGB = 1000*1024*1024;
        rlimit rl = { 2*kOneGB, 2*kOneGB };
        // set address space limit
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n",getpid());

    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof name - 1);
    AsyncLog log(::basename(name), kRollSize);
    log.start();
    g_asynclog = &log;

    bool long_log = argc > 1;
    bench(long_log);
}