//
// Time utility functions
// Extracted from util_time.cpp
// Original author: screemer
//
#include "time_utils.h"
#include <time.h>

using namespace std;

namespace TimeUtils {

time_t getCurrentTime() { return time(nullptr); }

// Returns true if using AB kernel and it used WiFi to update the current time
bool usingWiFiUpdatedTime() {
    time_t t = time(nullptr);
    tm *local = localtime(&t);

    return (local != nullptr) && (local->tm_year + 1900 >= 2020); // return true if the year >= 2020
}

// For custom datetimeformat from config.ini, callers should pass
// Gui::getInstance()->cfg.inifile.values["datetimeformat"] as the format parameter.
string timeToDisplayTimeString(time_t t, const string &_format) {
    string datetime;
    string format = _format; // if you pass a format it uses that

    if (format == "") {
        format = "%F %I:%M:%S %p"; // default: YYYY-MM-DD HH:MM:SS AM/PM
    }

    if (t != 0) {
        tm *local = localtime(&t);
        if ((local != nullptr) && (local->tm_year + 1900 >= 2020)) { // if datetime is from a WiFi enabled datetime
            char buf[200];
            if (std::strftime(buf, sizeof(buf), format.c_str(), local))
                datetime = buf;
        }
    }

    return datetime;
}

} // namespace TimeUtils
