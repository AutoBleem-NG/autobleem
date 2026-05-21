//
// Created by screemer on 2018-12-16.
//

#include "metadata.h"

#include <cstdio>

#include "../gui/gui.h"
#include "../utils/file_utils.h"
#include "../utils/string_utils.h"
#include "../environment.h"
#include "cover_db.h"
#include "rdb_reader.h"
#include "serial_scanner.h"

using namespace std;

namespace {

// Strip trailing parenthesized tokens like " (USA)", " (Disc 1)", " (Rev 1)".
// Mirrors RetroArch playlist label cleanup.
string cleanTitle(const string &name) {
    string s = name;
    while (true) {
        auto pos = s.rfind(" (");
        if (pos == string::npos)
            break;
        if (s.back() != ')')
            break;
        s.erase(pos);
    }
    return s;
}

// Map RetroArch's region string to AutoBleem's single-char region used by
// downstream code (pcsx.cfg generation, etc.). PAL countries collapse to "P",
// NTSC-J to "J", everything else to "U".
string mapRegion(const string &raRegion, const string &serial) {
    if (raRegion == "Japan")
        return "J";
    if (raRegion == "USA")
        return "U";
    if (raRegion == "Europe")
        return "P";
    // PAL countries shipped individually in the RA database.
    static const char *const pal[] = {"Australia", "France",      "Germany", "Italy",   "Spain",
                                      "Sweden",    "Netherlands", "Norway",  "Finland", "Denmark",
                                      "Portugal",  "Russia",      "Poland",  "Greece",  "UK"};
    for (const char *r : pal) {
        if (raRegion == r)
            return "P";
    }
    // Fallback to the serial-prefix heuristic.
    string fromSerial = SerialScanner::serialToRegion(serial);
    if (fromSerial == "Japan")
        return "J";
    if (fromSerial == "US")
        return "U";
    if (fromSerial == "Europe-Aus")
        return "P";
    return "U";
}

// Slurp a file into a freshly-allocated `new char[]` buffer. Returns nullptr
// if the file can't be opened or is empty. Caller takes ownership.
char *slurpFile(const string &path, int &outSize) {
    outSize = 0;
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
        return nullptr;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        return nullptr;
    }
    const size_t size = static_cast<size_t>(sz);
    char *buf = new char[size];
    size_t got = fread(buf, 1, size, f);
    fclose(f);
    if (got != size) {
        delete[] buf;
        return nullptr;
    }
    outSize = static_cast<int>(sz);
    return buf;
}

string thumbnailName(const string &name) { return FileUtils::replaceTheseCharsWithThisChar(name, "&*/:`<>?\\|", '_'); }

char *loadThumbnail(const string &name, int &outSize) {
    const string base = Env::getPathToRetroarchThumbnailsDir() + sep + "Sony - PlayStation" + sep + "Named_Boxarts" +
                        sep + thumbnailName(name);

    char *bytes = slurpFile(base + ".png", outSize);
    if (bytes)
        return bytes;
    return slurpFile(base + ".jpg", outSize);
}

void populate(Metadata *md, const RdbReader::Record &rec) {
    md->clean();
    md->title = cleanTitle(rec.name);
    md->publisher = rec.publisher;
    StringUtils::cleanPublisherString(md->publisher);
    md->year = rec.releaseyear;
    md->players = rec.users;
    md->serial = rec.serial;
    md->region = rec.region;
    md->lastRegion = mapRegion(rec.region, rec.serial);
    md->valid = true;

    // Load matching boxart. RetroArch's standard thumbnail layout:
    //   <thumbnails>/Sony - PlayStation/Named_Boxarts/<escaped-name>.png
    md->bytes = loadThumbnail(rec.name, md->dataSize);
}

} // namespace

bool Metadata::lookupBySerial(const string &serial) {
    shared_ptr<Gui> gui(Gui::getInstance());
    if (!gui->coverdb || !gui->coverdb->isValid())
        return false;
    const auto *rec = gui->coverdb->reader.findBySerial(serial);
    if (!rec)
        return false;
    populate(this, *rec);
    return true;
}

bool Metadata::lookupByTitle(const string &title) {
    shared_ptr<Gui> gui(Gui::getInstance());
    if (!gui->coverdb || !gui->coverdb->isValid())
        return false;
    const auto *rec = gui->coverdb->reader.findByName(title);
    if (!rec)
        return false;
    populate(this, *rec);
    return true;
}
