//
// Time utility functions
// Extracted from util_time.cpp
// Original author: screemer
//
#pragma once

#include <string>
#include <ctime>

namespace TimeUtils {

// Get current time as time_t
time_t getCurrentTime();

// Returns true if using AB kernel and it used WiFi to update the current time
// (year >= 2020 indicates proper time sync)
bool usingWiFiUpdatedTime();

// Convert time_t to formatted display string
// If format is empty, uses default: "%F %I:%M:%S %p" (YYYY-MM-DD HH:MM:SS AM/PM)
// For custom datetimeformat from config.ini, callers should pass
// Gui::getInstance()->cfg.inifile.values["datetimeformat"] as the format parameter.
std::string timeToDisplayTimeString(time_t t, const std::string &format = "");

} // namespace TimeUtils
