#ifndef NETLIBCC_CORE_LOGGER_H_
#define NETLIBCC_CORE_LOGGER_H_

#include "netlibcc/core/LogStream.h"
#include "netlibcc/core/TimeAnchor.h"

namespace netlibcc {

class Logger {
public:
    //  Use 6 Log levels
    enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

    // compile time calculation of basename of the source file
    struct SrcName {
        template<int N>
        SrcName(const char (&arr)[N]) : data_(arr), size_(N-1) {
            const char* slash = std::strrchr(data_, '/');
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SrcName(const char* filename) : data_(filename) {
            const char* slash = std::strrchr(filename, '/');
            if (slash) { data_ = slash + 1; }
            size_ = static_cast<int>(strlen(data_));
        }

        const char* data_;
        int size_;
    };

    // ctors
    Logger(SrcName fname, int lineno);
    Logger(SrcName fname, int lineno, LogLevel level);
    Logger(SrcName fname, int lineno, LogLevel level, const char* func);
    Logger(SrcName fname, int lineno, bool to_abort);

    ~Logger();

    using OutputFunc = void(*)(const char* msg, int len);
    using FlushFunc = void(*)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setLogLevel(LogLevel);
    static LogLevel getLogLevel();

    LogStream& stream() { return info_.stream_; }

private:
    struct LogInfo {
        LogInfo(Logger::LogLevel level, int old_errno, const SrcName& fname, int line);
        void finish();

        LogStream stream_;
        TimeAnchor time_;
        LogLevel level_;
        SrcName basename_;
        int line_;
    };

    // Logger data
    LogInfo info_;
};

extern Logger::LogLevel g_level;
inline Logger::LogLevel Logger::getLogLevel() {
    return g_level;
}

// convert errno to readable string info
const char* strerror_tl(int saved_errno);

// Log macros
#define LOG_TRACE if (netlibcc::Logger::getLogLevel() <= netlibcc::Logger::TRACE) \
    netlibcc::Logger(__FILE__, __LINE__, netlibcc::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (netlibcc::Logger::getLogLevel() <= netlibcc::Logger::DEBUG) \
    netlibcc::Logger(__FILE__, __LINE__, netlibcc::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (netlibcc::Logger::getLogLevel() <= netlibcc::Logger::INFO) \
    netlibcc::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN netlibcc::Logger(__FILE__, __LINE__, netlibcc::Logger::WARN).stream()
#define LOG_ERROR netlibcc::Logger(__FILE__, __LINE__, netlibcc::Logger::ERROR).stream()
#define LOG_FATAL netlibcc::Logger(__FILE__, __LINE__, netlibcc::Logger::FATAL).stream()
#define LOG_SYSERR netlibcc::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL netlibcc::Logger(__FILE__, __LINE__, true).stream()

} // namespace netlibcc

#endif // NETLIBCC_CORE_LOGGER_H_