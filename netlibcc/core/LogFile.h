#ifndef NETLIBCC_CORE_LOGFILE_H_
#define NETLIBCC_CORE_LOGFILE_H_

#include <memory>
#include "netlibcc/core/Mutex.h"
#include "netlibcc/core/FileAppender.h"

namespace netlibcc {

class LogFile : Noncopyable {
public:
    // ctor and dtor
    LogFile(const std::string& basename,
            off_t              rollsize,
            bool               locked    = true,
            int                flushgap  = 3,
            int                checkfreq = 1024);
    ~LogFile();

    void append(const char* str_line, int len);
    void flush();
    bool rollFile();

private:
    void unlockedAppend(const char*, int);
    static std::string getLogFileName(const std::string& basename, time_t* now);

    // basename_ is usually the process name
    const std::string basename_;

    // the file rolling size threshold
    const off_t rollsize_;

    // the buffer flush time threshold
    const int flushgap_;

    // every checkfreq_ times' append, we check if
    // a time period is reached, or the flush time gap
    // is longer than flush time threshold.
    const int checkfreq_;

    // append operation counter
    int count_;

    // states storage
    time_t start_period_;
    time_t last_roll_;
    time_t last_flush_;

    // data members (managed by smart pointer)
    std::unique_ptr<Mutex> mutex_;
    std::unique_ptr<FileAppender> appender_;

    // period of the file rolling time (seconds of a day)
    const static int kRollSeconds_ = 3600*24;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_LOGFILE_H_