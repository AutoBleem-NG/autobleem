#include "thumbnail_lookup.h"

#include "../environment.h"
#include "../utils/file_utils.h"

#include <vector>

namespace ThumbnailLookup {

namespace {

const std::vector<std::string> &imageExtensions() {
    static const std::vector<std::string> exts{".png", ".jpg", ".jpeg"};
    return exts;
}

} // namespace

std::string escapeName(const std::string &name) {
    return FileUtils::replaceTheseCharsWithThisChar(name, "&*/:`<>?\\|", '_');
}

std::string findThumbnailPath(const std::string &dbName, const std::string &title, const std::string &thumbnailDir) {
    if (dbName.empty() || title.empty() || thumbnailDir.empty())
        return "";

    const std::string base = Env::getPathToRetroarchThumbnailsDir() + sep +
                             FileUtils::getFileNameWithoutExtension(dbName) + sep + thumbnailDir + sep +
                             escapeName(title);
    for (const auto &ext : imageExtensions()) {
        const std::string path = base + ext;
        if (FileUtils::exists(path))
            return path;
    }
    return "";
}

std::string findBoxArtPath(const std::string &dbName, const std::string &title) {
    static const std::vector<std::string> dirs{"Named_Boxarts", "Named_Titles", "Named_Snaps"};
    for (const auto &dir : dirs) {
        const std::string path = findThumbnailPath(dbName, title, dir);
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

std::string findSnapPath(const std::string &dbName, const std::string &title, const std::string &romPath) {
    const std::string local = findLocalScreenshotPath(romPath);
    if (!local.empty())
        return local;
    return findThumbnailPath(dbName, title, "Named_Snaps");
}

} // namespace ThumbnailLookup
