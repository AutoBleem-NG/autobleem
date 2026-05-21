#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Reads RetroArch libretrodb (.rdb) files produced by RetroArch's
// libretro-database project. Parses the RARCHDB header and the
// sequence of rmsgpack-encoded record maps that follow, building
// in-memory indices keyed by `serial` and `name` for O(1) lookup.
//
// Designed for the small set of fields AutoBleem needs from the
// PS1 database (name, region, serial, publisher, users/players,
// releaseyear, genre). Only the rmsgpack subset that PS1 .rdb files
// actually use is supported.

class RdbReader {
  public:
    struct Record {
        std::string name;
        std::string region;
        std::string serial;
        std::string publisher;
        std::string developer;
        std::string genre;
        int releaseyear = 0;
        int releasemonth = 0;
        int users = 0;
    };

    RdbReader() = default;
    ~RdbReader() = default;

    bool open(const std::string &path);
    bool isValid() const { return valid; }

    const Record *findBySerial(const std::string &serial) const;
    const Record *findByName(const std::string &name) const;

    size_t size() const { return records.size(); }

  private:
    std::vector<Record> records;
    std::unordered_map<std::string, size_t> bySerial;
    std::unordered_map<std::string, size_t> byName;
    bool valid = false;
};
