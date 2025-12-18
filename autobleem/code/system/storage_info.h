// Storage information utilities
// Original author: screemer
#pragma once

#include <string>

namespace System {

// KB to GB conversion factor (1024 KB/MB * 1024 MB/GB)
constexpr long KB_PER_GB = 1024L * 1024L;

// Disk space information for USB storage device.
struct StorageInfo {
    float freeGB;
    float totalGB;
    int freePercent;

    StorageInfo() : freeGB(0.0f), totalGB(0.0f), freePercent(0) {}
    StorageInfo(float free, float total, int percent) : freeGB(free), totalGB(total), freePercent(percent) {}

    // Returns formatted string: "X.XX GB / Y.YY GB (Z%)"
    std::string formatted() const;
};

// Get storage info for the USB device mounted at /media.
// On x86/debug builds, returns zeros (stub for development).
// Note: path parameter is currently ignored on ARM - always reads /media.
StorageInfo getStorageInfo(const std::string &path = "/media");

} // namespace System
