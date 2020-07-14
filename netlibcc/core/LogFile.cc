#include "netlibcc/core/LogFile.h"

#include "unistd.h"
#include <ctime>
#include <cstdio>
#include <cassert>

namespace netlibcc {

// ctor
LogFile::LogFile(const std::string& basename,
                 off_t              rollsize,
                 bool               locked,
                 int                flushgap,
                 int                checkfreq)
            : basename_(basename),
              rollsize_(rollsize),
              flushgap_(flushgap),
              checkfreq_(checkfreq),
              count_(0),
              start_period_(0),
              last_roll_(0),
              last_flush_(0),
              mutex_(locked ? new Mutex : nullptr) {
    // make sure the basename is valid
    assert(basename.find('/') == std::string::npos);
    // generate the first file and do some initialization
    rollFile();
}

// default dtor is ok
LogFile::~LogFile() = default;

void LogFile::append(const char* str_line, int len) {
    if (mutex_) {
        MutexLockGuard lock(*mutex_);
        unlockedAppend(str_line, len);
    } else {
        unlockedAppend(str_line, len);
    }
}

void LogFile::flush() {
    if (mutex_) {
        MutexLockGuard lock(*mutex_);
        appender_->flush();
    } else {
        appender_->flush();
    }
}

bool LogFile::rollFile() {
    time_t now = 0;
    // get new filename and the current time (seconds)
    std::string filename = getLogFileName(basename_, &now);
    // get the number of whole days
    time_t start = now / kRollSeconds_ * kRollSeconds_;

    if (now > last_roll_) {
        last_roll_ = now;
        last_flush_ = now;
        start_period_ = start;

        // release the FileAppender obj and create a new one
        appender_.reset(new FileAppender(filename));
        return true;
    }
    return false;
}

// the filename format should be [basename].[create_time].[processId].log
std::string LogFile::getLogFileName(const std::string& basename, time_t* now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    // get the current time
    char timebuf[32];
    struct tm tmobj;
    *now = ::time(nullptr);
    gmtime_r(now, &tmobj);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tmobj);
    filename += timebuf;

    // get process Id
    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, ".%d", ::getpid());
    filename += pidbuf;

    filename += ".log";
    return filename;
}

void LogFile::unlockedAppend(const char* str_line, int len) {
    // every time write a log line
    appender_->append(str_line, len);

    // roll file size check
    if (appender_->writtenBytes() > rollsize_) {
        rollFile();
    } else {
        ++count_;
        if (count_ > checkfreq_) {
            count_ = 0;
            time_t now = ::time(nullptr);
            time_t this_period = now / kRollSeconds_ * kRollSeconds_;
            if (this_period != start_period_) {
                rollFile();
            } else if (now-last_flush_ > flushgap_) {
                last_flush_ = now;
                appender_->flush();
            }
        }
    }
}

} // namespace netlibcc