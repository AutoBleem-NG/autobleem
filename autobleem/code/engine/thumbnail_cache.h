#pragma once

#include <string>

class Coverdb;

struct ThumbnailCacheEntry {
    std::string recordName;
    std::string coverPath;
    std::string snapPath;
};

namespace ThumbnailCache {

ThumbnailCacheEntry findPlayStation(const Coverdb *coverdb, const std::string &serial, const std::string &title);

} // namespace ThumbnailCache
