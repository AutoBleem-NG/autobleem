// Storage information utilities
// Original author: screemer
#include "storage_info.h"

#include <stdexcept>

#include "process_utils.h"
#include "../utils/string_utils.h"

namespace System {

// Formats storage info as human-readable string for UI display.
// Example output: "12.34 GB / 64.00 GB (19%)"
std::string StorageInfo::formatted() const {
    return StringUtils::formatFloat(freeGB, 2) + " GB / " + StringUtils::formatFloat(totalGB, 2) + " GB (" +
           std::to_string(freePercent) + "%)";
}

// Retrieves disk space information for the USB storage device.
// On PSC hardware: Parses 'df' command output to get /media mount stats.
// On x86/debug builds: Returns zeros (stub for development without real USB).
//
// Note: The path parameter is currently ignored on ARM - the PSC only supports
// a single USB storage device which is always mounted at /media.
StorageInfo getStorageInfo([[maybe_unused]] const std::string &path) {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    // Development stub - no real USB device available
    return StorageInfo{0.0f, 0.0f, 0};
#else
    // Parse df output for media mount point.
    // df output format: "Filesystem 1K-blocks Used Available Use% Mounted"
    // We extract $4 (available KB) and $2 (total KB) from the /media mount line.
    std::string freeStr = executeCommand("df | grep \"media\" | head -1 | awk '{print $4}'");
    std::string totalStr = executeCommand("df | grep \"media\" | head -1 | awk '{print $2}'");

    float freeSpace = 0.0f;
    float totalSpace = 0.0f;

    try {
        if (!freeStr.empty()) {
            // Use stol (long) instead of stoi (int) because KB values can exceed
            // INT_MAX on large drives. A 2TB drive = 2,147,483,647 KB which is
            // exactly INT_MAX, so anything larger would overflow.
            freeSpace = static_cast<float>(std::stol(freeStr)) / KB_PER_GB;
        }
        if (!totalStr.empty()) {
            totalSpace = static_cast<float>(std::stol(totalStr)) / KB_PER_GB;
        }
    } catch (const std::exception &) {
        // Return zeros on parse failure rather than crashing.
        // Storage display is non-critical UI element - showing "0 GB" is better
        // than crashing if df output is unexpected or malformed.
        return StorageInfo{0.0f, 0.0f, 0};
    }

    // Calculate percentage, avoiding division by zero
    int freeSpacePerc = 0;
    if (totalSpace > 0.0f) {
        freeSpacePerc = static_cast<int>((freeSpace / totalSpace) * 100);
    }

    return StorageInfo{freeSpace, totalSpace, freeSpacePerc};
#endif
}

} // namespace System
