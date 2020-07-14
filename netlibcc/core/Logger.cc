#include "netlibcc/core/Logger.h"
#include <errno.h>
#include <cstdio>
#include "netlibcc/core/ThisThread.h"

namespace netlibcc {

// TLS to save errno info
__thread char t_errnobuf[512];

const char* strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, t_errnobuf, sizeof t_errnobuf);
}

void defaultOutput(const char* msg, int len) {
    fwrite(msg, 1, len, stdout);
}

void defaultFlush() {
    fflush(stdout);
}

Logger::LogLevel g_level = Logger::INFO;
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

// fixed level name, whose size is always 6 bytes
const char* level_name[6] = { "TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ", };

// Logger::LogInfo ctor
Logger::LogInfo::LogInfo(Logger::LogLevel level, int old_errno, const SrcName& fname, int line)
                    : stream_(),
                      time_(TimeAnchor::now()),
                      level_(level),
                      basename_(fname),
                      line_(line) {
    // format time (UTC time stamp)
    stream_ << time_.formatMicro() << ' ';
    // cache tid if it is needed
    thisthread::tid();
    // write cached tid info
    stream_.append(thisthread::tidStr(), thisthread::tidStrLen());
    // write current log level
    stream_.append(level_name[level], 6);
    // if we use LOG_SYSERR or LOG_SYSFATAL
    if (old_errno) {
        stream_ << strerror_tl(old_errno) << " (errno=" << old_errno << ") ";
    }
}

void Logger::LogInfo::finish() {
    stream_ << " - ";
    stream_.append(basename_.data_, basename_.size_);
    stream_ << ':' << line_ << '\n';
}

// Logger ctors
Logger::Logger(SrcName fname, int lineno) : info_(INFO, 0, fname, lineno) {}
Logger::Logger(SrcName fname, int lineno, LogLevel level) : info_(level, 0, fname, lineno) {}
Logger::Logger(SrcName fname, int lineno, LogLevel level, const char* func)
            : info_(level, 0, fname, lineno) {
    info_.stream_ << func << ' ';
}
Logger::Logger(SrcName fname, int lineno, bool to_abort)
            : info_(to_abort ? FATAL : ERROR, errno, fname, lineno) {}

// Logger dtor
Logger::~Logger() {
    info_.finish();
    const LogStream::Buffer& residual_buf(stream().buffer());
    // residual buffer bytes output by pre-defined g_output method
    g_output(residual_buf.data(), residual_buf.length());
    if (info_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::setOutput(OutputFunc out) {
    g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
    g_flush = flush;
}

void Logger::setLogLevel(LogLevel level) {
    g_level = level;
}

} // namespace netlibcc