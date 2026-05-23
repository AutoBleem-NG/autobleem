#pragma once

#include <string>

// Disc-suffix parser used by the scanner to detect multi-disc PSX game
// directories. Recognises the common naming conventions emitted by ROM
// managers (Redump, no-intro, libretro playlists):
//
//   "Game (Disc 1)"   "Game (Disk 2)"   "Game (CD 3)"   "Game (CD3)"
//   "Game - Disc 1"   "Game - Disk 2"   "Game - CD 3"
//
// Matching is case-insensitive on the keyword. Trailing whitespace is
// tolerated. The leading base name is returned with any trailing
// whitespace before the marker stripped.

struct DiscSuffix {
    std::string base; // Game name without the disc marker.
    int disc = 0;     // Disc number, or 0 if no marker was found.

    bool matched() const { return disc != 0; }
};

DiscSuffix parseDiscSuffix(const std::string &name);
