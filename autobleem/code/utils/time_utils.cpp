//
// Time utility functions
// Extracted from util_time.cpp
// Original author: screemer
//
#include "time_utils.h"

#include <ctime>

namespace TimeUtils {

// The PlayStation Classic hardware clock resets to approximately 1970 or 2000
// when power is lost. When the AutoBleem kernel has WiFi connectivity, it syncs
// the system time via NTP. We use year >= 2020 as a heuristic to detect whether
// the time has been properly synchronized (since PSC was released in 2018).
static constexpr int MIN_VALID_YEAR = 2020;

time_t getCurrentTime() { return std::time(nullptr); }

bool usingWiFiUpdatedTime() {
    time_t t = std::time(nullptr);
    std::tm *local = std::localtime(&t);

    return (local != nullptr) && (local->tm_year + 1900 >= MIN_VALID_YEAR);
}

std::string timeToDisplayTimeString(time_t t, const std::string &format) {
    // Use caller's format, or default to ISO date with 12-hour time
    std::string effectiveFormat = format.empty() ? "%F %I:%M:%S %p" : format;

    if (t == 0) {
        return "";
    }

    std::tm *local = std::localtime(&t);
    if (local == nullptr || local->tm_year + 1900 < MIN_VALID_YEAR) {
        // Time is invalid or predates NTP sync (PSC clock not set properly)
        return "";
    }

    char buf[200];
    if (std::strftime(buf, sizeof(buf), effectiveFormat.c_str(), local) == 0) {
        return "";
    }

    return buf;
}

} // namespace TimeUtils
