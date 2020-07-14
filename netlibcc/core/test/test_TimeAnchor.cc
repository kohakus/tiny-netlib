#include "netlibcc/core/TimeAnchor.h"
#include <cstdio>

using namespace netlibcc;

int main() {
    TimeAnchor start(TimeAnchor::now());
    printf("now -> %s\n", start.toStr().c_str());
    printf("The micro format time: %s\n", start.formatMicro().c_str());
    printf("The seconds format time: %s\n", start.formatSeconds().c_str());

    // some busy task
    for (int i = 0; i < 10000; i++) {}

    // show time interval
    printf("now time interval: %f\n", timeDiff(TimeAnchor::now(), start));
}