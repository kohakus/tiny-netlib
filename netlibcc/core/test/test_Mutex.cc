#include <vector>
#include <cstdio>

#include "netlibcc/core/Mutex.h"
#include "netlibcc/core/Thread.h"
#include "netlibcc/core/TimeAnchor.h"

using namespace netlibcc;
using namespace std;

Mutex g_mutex;
vector<int> g_vec;
const int kCount = 10000000;

// every thread push kCount values into g_vec
void threadFunc() {
    MutexLockGuard lock(g_mutex);
    for (int i = 0; i < kCount; i++) {
        g_vec.push_back(i);
    }
}

int main() {
    const int kMaxThreads = 8;
    g_vec.reserve(kCount * kMaxThreads);
    for (int nthreads = 1; nthreads <= kMaxThreads; nthreads++) {
        vector<unique_ptr<Thread>> threads;
        g_vec.clear();

        auto start = TimeAnchor::now();
        // create thread objs and start every thread
        for (int i = 0; i < nthreads; i++) {
            threads.emplace_back(new Thread(threadFunc));
            threads.back()->start();
        }
        for (int i = 0; i < nthreads; i++) {
            threads[i]->join();
        }
        printf("%d thread(s) with lock, time cost: %f\n", nthreads, timeDiff(TimeAnchor::now(), start));

        // check order
        for (int i = 0; i < nthreads; i++) {
            for (int j = i*kCount, k = 0; k < kCount; k++, j++) {
                assert(k == g_vec[j]);
            }
        }
    }
    printf("Totally %d threads created.\n", Thread::getNumCreated());
}