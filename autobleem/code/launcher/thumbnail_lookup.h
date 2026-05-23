#pragma once

#include <string>

// Resolves the on-disk paths to images served by RetroArch's
// libretro-thumbnails packs. Filesystem-only — no game/playlist context
// required, which keeps the module trivially unit-testable.
//
// Layout assumed:
//   <RA>/thumbnails/<system>/Named_Boxarts/<escaped-name>.{png,jpg,jpeg}
//   <RA>/thumbnails/<system>/Named_Titles/...
//   <RA>/thumbnails/<system>/Named_Snaps/...
//   <RA>/screenshots/<rom-base>(-YYMMDD-HHMMSS)?.{png,jpg,jpeg}
//   <RA>/states/<rom-base>.state.auto.png

namespace ThumbnailLookup {

// libretro-thumbnails replaces filesystem-unsafe characters in record names
// with '_' before writing files. Exposed so callers building their own
// candidates can match the same escaping.
std::string escapeName(const std::string &name);

// Looks up `<RA>/thumbnails/<dbName-stem>/<thumbnailDir>/<escaped-title>.{ext}`
// in PNG → JPG → JPEG order. Returns the first existing path or "".
std::string findThumbnailPath(const std::string &dbName, const std::string &title, const std::string &thumbnailDir);

// Looks up Named_Boxarts first, falling back to Named_Titles then
// Named_Snaps so a missing boxart still produces *some* image.
std::string findBoxArtPath(const std::string &dbName, const std::string &title);

// Returns the most recent screenshot saved by RetroArch for `romPath`'s
// basename, preferring <RA>/screenshots/ then the savestate auto-thumbnail.
// Returns "" if no local screenshot exists.
std::string findLocalScreenshotPath(const std::string &romPath);

// Prefers a user-captured screenshot (findLocalScreenshotPath) over the
// canned libretro-thumbnails Named_Snaps image.
std::string findSnapPath(const std::string &dbName, const std::string &title, const std::string &romPath = "");

} // namespace ThumbnailLookup
