//
// AutoBleem Logging
//
// Usage:
//   #include "log.h"
//
//   // Initialize once at startup (in main.cpp)
//   Log::init();
//
//   // Then use anywhere:
//   PLOG_DEBUG << "Debug message: " << value;
//   PLOG_INFO << "Info message";
//   PLOG_WARNING << "Warning message";
//   PLOG_ERROR << "Error message";
//
// Log files are written to USB:/System/Logs/autobleem-ng.log on device,
// or ./autobleem-ng.log during local development.
// NOTE: Log::init() must be called AFTER environment paths are set up in main()
//

#pragma once

#include <iomanip>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Util.h>

#include "utils/file_utils.h"

namespace Log {

// Custom formatter: "12:32:13 INFO  [main:215] message"
// Compact format without date and thread ID for embedded use
class Formatter {
  public:
    static plog::util::nstring header() { return plog::util::nstring(); }

    static plog::util::nstring format(const plog::Record &record) {
        tm t;
        plog::util::localtime_s(&t, &record.getTime().time);

        plog::util::nostringstream ss;
        ss << std::setfill('0') << std::setw(2) << t.tm_hour << ":" << std::setfill('0') << std::setw(2) << t.tm_min
           << ":" << std::setfill('0') << std::setw(2) << t.tm_sec << " " << std::setfill(' ') << std::setw(5)
           << std::left << plog::severityToString(record.getSeverity()) << " [" << record.getFunc() << ":"
           << record.getLine() << "] " << record.getMessage() << "\n";

        return ss.str();
    }
};

// Default log level - can be changed at compile time with -DLOG_LEVEL=...
#ifndef LOG_LEVEL
#ifdef NDEBUG
#define LOG_LEVEL plog::info // Release: info and above
#else
#define LOG_LEVEL plog::debug // Debug: debug and above
#endif
#endif

// Initialize logging - call once at startup
// logPath: optional path to log file (if not provided, uses "./autobleem-ng.log")
inline void init(const std::string &logPath = "autobleem-ng.log", plog::Severity maxSeverity = LOG_LEVEL) {
    FileUtils::ensureParentDirExists(logPath);

    static plog::RollingFileAppender<Formatter> fileAppender(logPath.c_str(), 1024 * 1024, 3);
    static plog::ColorConsoleAppender<Formatter> consoleAppender;

    plog::init(maxSeverity, &fileAppender).addAppender(&consoleAppender);

    PLOG_INFO << "=== AutoBleem started ===";
    PLOG_INFO << "Log file: " << logPath;
}

// Initialize file-only logging for utilities (no console output)
// Useful for background processes that don't have a terminal
inline void initFileOnly(const std::string &logPath, size_t maxFileSize = 100 * 1024, int maxFiles = 2,
                         plog::Severity maxSeverity = LOG_LEVEL) {
    FileUtils::ensureParentDirExists(logPath);

    static plog::RollingFileAppender<Formatter> fileAppender(logPath.c_str(), maxFileSize, maxFiles);
    plog::init(maxSeverity, &fileAppender);
}

// Change log level at runtime
inline void setLevel(plog::Severity severity) {
    plog::get()->setMaxSeverity(severity);
    PLOG_INFO << "Log level changed to: " << plog::severityToString(severity);
}

} // namespace Log
