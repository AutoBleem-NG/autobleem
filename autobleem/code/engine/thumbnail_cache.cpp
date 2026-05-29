#include "thumbnail_cache.h"

#include "cover_db.h"
#include "../launcher/thumbnail_lookup.h"

namespace ThumbnailCache {

ThumbnailCacheEntry findPlayStation(const Coverdb *coverdb, const std::string &serial, const std::string &title) {
    ThumbnailCacheEntry entry;
    entry.recordName = Coverdb::findRecordNameForSerial(coverdb, serial);
    entry.coverPath = ThumbnailLookup::findBoxArtPath(ThumbnailLookup::PlayStationDbName, title, entry.recordName);
    entry.snapPath = ThumbnailLookup::findSnapPath(ThumbnailLookup::PlayStationDbName, title, "", entry.recordName);
    return entry;
}

} // namespace ThumbnailCache
