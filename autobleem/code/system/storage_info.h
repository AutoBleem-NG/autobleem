//
// Storage information utilities
// Extracted from util.cpp (getAvailableSpace)
// Original author: screemer
//
#pragma once

#include <string>

namespace System {

// Disk space information for USB device
struct StorageInfo {
    float freeGB;
    float totalGB;
    int freePercent;

    std::string formatted() const;
};

// Return the available space of a USB device
StorageInfo getStorageInfo(const std::string &path = "/media");

} // namespace System
