//
// Storage information utilities
// Extracted from util.cpp (getAvailableSpace)
// Original author: screemer
//
#include "storage_info.h"

#include <cstdlib>

#include "process_utils.h"
#include "../utils/string_utils.h"

using namespace std;

namespace System {

string StorageInfo::formatted() const {
    return StringUtils::formatFloat(freeGB, 2) + " GB / " + StringUtils::formatFloat(totalGB, 2) + " GB (" +
           to_string(freePercent) + "%)";
}

StorageInfo getStorageInfo(const string &path) {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    return StorageInfo{0, 0, 0};
#else
    int gb = 1024 * 1024;
    float freeSpace = ((float)(stoi(executeCommand("df | grep \"media\" | head -1 | awk '{print $4}'")))) / gb;
    float totalSpace = ((float)(stoi(executeCommand("df | grep \"media\" | head -1 | awk '{print $2}'")))) / gb;
    int freeSpacePerc = (freeSpace / totalSpace) * 100;

    return StorageInfo{freeSpace, totalSpace, freeSpacePerc};
#endif
}

} // namespace System
