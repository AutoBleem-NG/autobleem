# Bundled Libraries

This directory contains third-party libraries bundled with AutoBleem to simplify the build process and ensure consistent behavior across different build environments.

## Libraries

### libchdr
**Source:** https://github.com/rtissera/libchdr
**Commit:** `8bba7745d758627258b315997a860039244cedaf` (2025-06-08)

CHD (Compressed Hunks of Data) reading library, originally from MAME. Provides support for reading CHD disc images, which are commonly used for archiving PlayStation game discs.

Note: libchdr does not publish versioned releases. The commit hash above identifies the exact version bundled.

**Why bundled:**
- libchdr requires the LZMA SDK (not the standard liblzma), which has different headers than what's typically available in system packages
- Bundling ensures consistent CHD support without requiring users to build and install libchdr system-wide
- ARM cross-compilation is simplified by having the source available

**Included dependencies:**
- LZMA SDK 24.05
- zlib 1.3.1
- zstd 1.5.6

**Usage:**
- Local x86_64 builds: Built as part of the CMake project, linked statically
- ARM/Docker builds: Pre-built static library from cross-tools (built in Dockerfile)

### nlohmann/json
**Source:** https://github.com/nlohmann/json
**Version:** 3.6.1

Header-only JSON library for modern C++. Used for parsing and generating JSON configuration files.

**Why bundled:**
- Header-only library, trivial to include
- Ensures consistent JSON parsing behavior across all build environments

### sqlite3
**Source:** https://sqlite.org/
**Header (sqlite3ab.h):** Official SQLite 3.26.0 (2018-12-01)
- SOURCE_ID: `2018-12-01 12:34:55 bf8c1b2b7a5960c282e543b9c293686dccff272512d08865f4600fb58238b4f9`
- Matches [azadkuh/sqlite-amalgamation tag 3.26.0](https://github.com/azadkuh/sqlite-amalgamation/tree/3.26.0)

**Implementation (sqlite3ab.c):** Development snapshot (2019-03-02) - **NOT an official release**
- Claims version "3.28.0" but is actually from SQLite trunk between 3.27.2 and official 3.28.0 release
- SOURCE_ID: `2019-03-02 15:25:24 fd085e9260bec18f968704abb2dd324d954baa121d13b67c3f5b801e9e3834aa`
- Corresponds to [fossil commit fd085e9260](https://www.sqlite.org/src/info/fd085e9260bec18f)
- Official 3.28.0 was released on 2019-04-16 with different SOURCE_ID

**Known Issue:** Header and implementation files are mismatched (3.26.0 vs development 3.28.0). This could cause compatibility issues. Consider updating to matching official releases from [azadkuh/sqlite-amalgamation](https://github.com/azadkuh/sqlite-amalgamation/tags).

SQLite database engine. Used for game metadata storage, cover art databases, and play history.

**Why bundled:**
- Amalgamation build (single .c/.h file) simplifies cross-compilation
- Ensures consistent database format across PSC hardware and development builds
- "ab" suffix is a naming convention to distinguish from system sqlite3 (no code modifications)

## Updating Libraries

When updating bundled libraries:

1. **libchdr:** Clone the repository and copy `src/`, `include/`, `deps/`, `CMakeLists.txt`, and `LICENSE.txt`. Remove the `add_subdirectory(tests)` line from CMakeLists.txt. Update the commit hash in this README.

2. **nlohmann/json:** Download the single-header version (`json.hpp`) and rename to `json.h`. Update version in this README.

3. **sqlite3:** Use matching versions from [azadkuh/sqlite-amalgamation](https://github.com/azadkuh/sqlite-amalgamation/tags) to ensure header and implementation match. Download `sqlite3.h` and `sqlite3.c`, rename to `sqlite3ab.h` and `sqlite3ab.c`. Verify SOURCE_ID matches between both files. Update version and SOURCE_ID in this README.

## License Information

Each library retains its original license:
- libchdr: BSD-3-Clause (see `libchdr/LICENSE.txt`)
- nlohmann/json: MIT License (see header in `nlohmann/json.h`)
- sqlite3: Public Domain
- LZMA SDK: Public Domain
- zlib: zlib License (see `libchdr/deps/zlib-*/LICENSE`)
- zstd: BSD License (see `libchdr/deps/zstd-*/LICENSE`)
