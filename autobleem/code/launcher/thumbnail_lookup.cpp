#include "thumbnail_lookup.h"

#include "../environment.h"
#include "../utils/file_utils.h"

#include <cctype>
#include <dirent.h>
#include <unordered_map>
#include <vector>

namespace ThumbnailLookup {

namespace {

const std::vector<std::string> &imageExtensions() {
    static const std::vector<std::string> exts{".png", ".jpg", ".jpeg"};
    return exts;
}

// Cache directory listings keyed by absolute path. The PS1 boxarts dir has
// ~8000 entries; without caching the fuzzy fallback would re-readdir for every
// missed game during launcher startup. clearCaches() exposes invalidation for
// tests that mutate the thumbnails tree between calls.
std::unordered_map<std::string, std::vector<std::string>> &dirCache() {
    static std::unordered_map<std::string, std::vector<std::string>> c;
    return c;
}

const std::vector<std::string> &listDir(const std::string &dir) {
    auto &cache = dirCache();
    auto it = cache.find(dir);
    if (it != cache.end())
        return it->second;

    // Use opendir/readdir directly rather than FileUtils::diru_FilesOnly: that
    // helper stat()s every entry to populate isDir, which is catastrophically
    // slow on the PSC's USB IO when the dir holds ~8000 libretro-thumbnails
    // files (tens of seconds per dir, looks like a hang to the user).
    std::vector<std::string> names;
    DIR *dp = ::opendir(dir.c_str());
    if (dp != nullptr) {
        struct dirent *entry;
        while ((entry = ::readdir(dp)) != nullptr) {
            if (entry->d_name[0] == '.')
                continue;
            // Filter only entries we *know* are directories — exFAT/FAT often
            // reports DT_UNKNOWN, in which case we accept and let the later
            // image-extension filter decide.
            if (entry->d_type == DT_DIR)
                continue;
            names.emplace_back(entry->d_name);
        }
        ::closedir(dp);
    }
    return cache.emplace(dir, std::move(names)).first->second;
}

} // namespace

std::string escapeName(const std::string &name) {
    return FileUtils::replaceTheseCharsWithThisChar(name, "&*/:`<>?\\|", '_');
}

namespace {

// Try one candidate name plus each shorter form produced by peeling off
// trailing " (...)" parenthesised tags. Returns the first hit on disk or "".
std::string tryWithTagStripping(const std::string &dir, std::string candidate) {
    while (!candidate.empty()) {
        const std::string base = dir + escapeName(candidate);
        for (const auto &ext : imageExtensions()) {
            const std::string path = base + ext;
            if (FileUtils::exists(path))
                return path;
        }
        const auto pos = candidate.rfind(" (");
        if (pos == std::string::npos || candidate.back() != ')')
            return "";
        candidate.erase(pos);
    }
    return "";
}

// Pull each trailing parenthesised tag out of `name`: "Foo (USA) (Rev 1)" →
// ["(USA)", "(Rev 1)"]. Only well-formed " (...)" suffixes are extracted —
// stray parens in the middle of the name are left alone.
std::vector<std::string> extractTrailingTags(const std::string &name) {
    std::vector<std::string> tags;
    std::string remaining = name;
    while (!remaining.empty() && remaining.back() == ')') {
        const auto pos = remaining.rfind(" (");
        if (pos == std::string::npos)
            break;
        tags.push_back(remaining.substr(pos + 1));
        remaining.erase(pos);
    }
    return tags;
}

// Strip all trailing " (...)" tags to get the bare base name.
std::string stripTrailingTags(std::string name) {
    while (!name.empty() && name.back() == ')') {
        const auto pos = name.rfind(" (");
        if (pos == std::string::npos)
            break;
        name.erase(pos);
    }
    return name;
}

bool hasImageExtension(const std::string &name) {
    for (const auto &ext : imageExtensions()) {
        if (FileUtils::matchExtension(name, ext))
            return true;
    }
    return false;
}

// Last-resort fuzzy match: scan the directory for any file named exactly
// "<bare> (...)<ext>" and pick the closest variant. The literal " (" suffix on
// the prefix ensures we don't match unrelated games — e.g. looking for "Doom"
// won't pull in "Doom 2 - Hell on Earth (USA).jpg".
//
// Among candidates, score by:
//   * overlap with the caller's original parenthesised tags (×10) — so a user
//     with `Suikoden (USA)` picks `Suikoden (USA) (Rev 1).jpg` over
//     `Suikoden (Europe).jpg`;
//   * region preference (USA > Europe > World) as a mild tiebreaker;
//   * lexicographic order for full determinism.
std::string fuzzyMatch(const std::string &dir, const std::string &bare, const std::vector<std::string> &preferredTags) {
    if (bare.empty())
        return "";
    const std::string prefix = escapeName(bare) + " (";

    std::string best;
    int bestScore = -1;
    for (const auto &name : listDir(dir)) {
        if (name.size() <= prefix.size())
            continue;
        if (name.compare(0, prefix.size(), prefix) != 0)
            continue;
        if (!hasImageExtension(name))
            continue;

        int score = 0;
        for (const auto &tag : preferredTags) {
            if (!tag.empty() && name.find(tag) != std::string::npos)
                score += 10;
        }
        if (name.find("(USA)") != std::string::npos)
            score += 3;
        else if (name.find("(Europe)") != std::string::npos)
            score += 2;
        else if (name.find("(World)") != std::string::npos)
            score += 1;

        if (score > bestScore || (score == bestScore && (best.empty() || name < best))) {
            bestScore = score;
            best = name;
        }
    }
    return best.empty() ? "" : dir + best;
}

} // namespace

std::string findThumbnailPath(const std::string &dbName, const std::string &title, const std::string &thumbnailDir,
                              const std::string &recordName) {
    if (dbName.empty() || thumbnailDir.empty())
        return "";

    const std::string dir = Env::getPathToRetroarchThumbnailsDir() + sep +
                            FileUtils::getFileNameWithoutExtension(dbName) + sep + thumbnailDir + sep;

    if (!recordName.empty()) {
        const std::string hit = tryWithTagStripping(dir, recordName);
        if (!hit.empty())
            return hit;
    }
    if (!title.empty()) {
        const std::string hit = tryWithTagStripping(dir, title);
        if (!hit.empty())
            return hit;
    }

    // Fuzzy fallback: bare name + " (anything)". recordName is preferred for
    // the bare query since it's the canonical libretro form when available.
    const std::string &fuzzySource = !recordName.empty() ? recordName : title;
    if (fuzzySource.empty())
        return "";
    std::vector<std::string> preferredTags = extractTrailingTags(fuzzySource);
    if (!recordName.empty() && !title.empty() && recordName != title) {
        for (auto &t : extractTrailingTags(title))
            preferredTags.push_back(std::move(t));
    }
    return fuzzyMatch(dir, stripTrailingTags(fuzzySource), preferredTags);
}

std::string findBoxArtPath(const std::string &dbName, const std::string &title, const std::string &recordName) {
    static const std::vector<std::string> dirs{"Named_Boxarts", "Named_Titles", "Named_Snaps"};
    for (const auto &dir : dirs) {
        const std::string path = findThumbnailPath(dbName, title, dir, recordName);
        if (!path.empty())
            return path;
    }
    return "";
}

std::string findLocalScreenshotPath(const std::string &romPath) {
    if (romPath.empty())
        return "";

    const std::string base = FileUtils::getFileNameWithoutExtension(FileUtils::getFileNameFromPath(romPath));
    if (base.empty())
        return "";

    const std::string screenshotsDir = Env::getPathToRetroarchScreenshotsDir();
    if (FileUtils::exists(screenshotsDir)) {
        // RetroArch writes either "<base>.png" or "<base>-YYMMDD-HHMMSS.png".
        // Timestamped names sort lexicographically by recency, so the last
        // match is the newest without needing stat() calls.
        std::string newest;
        for (const auto &entry : FileUtils::diru_FilesOnly(screenshotsDir)) {
            if (entry.name.size() <= base.size() || entry.name.compare(0, base.size(), base) != 0)
                continue;
            const char next = entry.name[base.size()];
            if (next != '.' && next != '-')
                continue;
            bool extOk = false;
            for (const auto &ext : imageExtensions()) {
                if (FileUtils::matchExtension(entry.name, ext)) {
                    extOk = true;
                    break;
                }
            }
            if (extOk && entry.name > newest)
                newest = entry.name;
        }
        if (!newest.empty())
            return screenshotsDir + sep + newest;
    }

    const std::string stateThumb = Env::getPathToRetroarchStatesDir() + sep + base + ".state.auto.png";
    if (FileUtils::exists(stateThumb))
        return stateThumb;
    return "";
}

std::string findSnapPath(const std::string &dbName, const std::string &title, const std::string &romPath,
                         const std::string &recordName) {
    const std::string local = findLocalScreenshotPath(romPath);
    if (!local.empty())
        return local;
    return findThumbnailPath(dbName, title, "Named_Snaps", recordName);
}

void clearCaches() { dirCache().clear(); }

} // namespace ThumbnailLookup
