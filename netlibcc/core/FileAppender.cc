#include "netlibcc/core/FileAppender.h"
#include <cassert>
#include <cstdio>
#include "netlibcc/core/Logger.h"

namespace netlibcc {

// open a file with append mode, 'e' for O_CLOEXEC
FileAppender::FileAppender(StringArg fname)
        : fp_(::fopen(fname.c_str(), "ae")),
          written_bytes_(0) {
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);
}

FileAppender::~FileAppender() {
    ::fclose(fp_);
}

void FileAppender::flush() {
    ::fflush(fp_);
}

// unlocked write, not thread safe
size_t FileAppender::unlockedWrite(const char* str_line, size_t len) {
    return ::fwrite_unlocked(str_line, 1, len, fp_);
}

void FileAppender::append(const char* str_line, size_t len) {
    size_t written_n = unlockedWrite(str_line, len);
    size_t residual_n = len - written_n;

    // make sure a log line is written completely
    while (residual_n > 0) {
        // try to write residual bytes
        size_t again_n = unlockedWrite(str_line + written_n, residual_n);
        if (again_n == 0) {
            // check error
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "FileAppender::append() failed %s\n", strerror_tl(err));
            }
            break;
        }
        written_n += again_n;
        residual_n = len - written_n;
    }
    written_bytes_ += len;
}

} // namespace netlibcc