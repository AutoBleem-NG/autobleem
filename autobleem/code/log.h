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
// Log files are written to /media/System/Logs/autobleem.log on device,
// or ./autobleem.log during local development.
//

#pragma once

#include <iomanip>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Util.h>

#include "dir_entry.h"

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

// Log file paths
inline const char *getLogPath() {
    // On PlayStation Classic, logs go to /media/System/Logs/
    // For local development, use current directory
    static const char *devicePath = "/media/System/Logs/autobleem.log";
    static const char *localPath = "autobleem.log";

    if (DirEntry::exists("/media/System/Logs")) {
        return devicePath;
    }
    return localPath;
}

// Initialize logging - call once at startup
inline void init(plog::Severity maxSeverity = LOG_LEVEL) {
    static plog::RollingFileAppender<Formatter> fileAppender(getLogPath(), 1024 * 1024, 3);
    static plog::ColorConsoleAppender<Formatter> consoleAppender;

    plog::init(maxSeverity, &fileAppender).addAppender(&consoleAppender);

    PLOG_INFO << "=== AutoBleem started ===";
}

// Change log level at runtime
inline void setLevel(plog::Severity severity) {
    plog::get()->setMaxSeverity(severity);
    PLOG_INFO << "Log level changed to: " << plog::severityToString(severity);
}

} // namespace Log
