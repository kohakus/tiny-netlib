#ifndef NETLIBCC_CORE_FILEAPPENDER_H_
#define NETLIBCC_CORE_FILEAPPENDER_H_

#include <sys/types.h>
#include "netlibcc/core/Noncopyable.h"
#include "netlibcc/core/StringArg.h"

namespace netlibcc {

// RAII class to manage file pointer and write operation
class FileAppender {
public:
    explicit FileAppender(StringArg fname);
    ~FileAppender();

    void append(const char* str_line, size_t len);

    void flush();

    off_t writtenBytes() const {
        return written_bytes_;
    }

private:
    // unlocked write, not thread safe
    size_t unlockedWrite(const char* str_line, size_t len);
    // self defined file write buffer, 64K
    char buffer_[64*1024];
    FILE* fp_;
    // the number of bytes have written
    off_t written_bytes_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_FILEAPPENDER_H_