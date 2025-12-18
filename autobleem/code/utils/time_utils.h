//
// Time utility functions
// Extracted from util_time.cpp
// Original author: screemer
//
#pragma once

#include <ctime>
#include <string>

// Time utilities for the PlayStation Classic.
// The PSC hardware clock resets when power is lost, so these functions
// help detect whether the system time has been properly synchronized
// via WiFi/NTP (available with the AutoBleem kernel).

namespace TimeUtils {

// Returns the current system time as a Unix timestamp.
[[nodiscard]] time_t getCurrentTime();

// Returns true if the system time appears to be properly synchronized.
// The PSC clock resets to ~1970/2000 on power loss. When the AutoBleem kernel
// has WiFi connectivity, it syncs time via NTP. We use year >= 2020 as a
// heuristic to detect valid time (PSC was released in 2018).
[[nodiscard]] bool usingWiFiUpdatedTime();

// Converts a Unix timestamp to a formatted display string.
// Returns empty string if:
//   - t is 0 (no time set)
//   - The year is before 2020 (indicates PSC clock not synced)
// If format is empty, uses default: "%F %I:%M:%S %p" (YYYY-MM-DD HH:MM:SS AM/PM)
// For custom datetime format from config.ini, pass the user's configured format.
[[nodiscard]] std::string timeToDisplayTimeString(time_t t, const std::string &format = "");

} // namespace TimeUtils
