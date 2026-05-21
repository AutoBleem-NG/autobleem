//
// Created by screemer on 2/2/19.
//
#pragma once

#include "rdb_reader.h"

//******************
// Coverdb
//
// Wraps the RetroArch libretrodb (.rdb) for Sony - PlayStation. Replaces the
// previous SQLite-backed regional cover databases — metadata is now read from
// the RetroArch DB and cover art from the matching boxart JPG.
//******************
class Coverdb {
  public:
    RdbReader reader;

    Coverdb();
    ~Coverdb() = default;

    bool isValid() const { return reader.isValid(); }
};
