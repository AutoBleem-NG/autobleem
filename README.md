# AutoBleem-NG

[![CI](https://github.com/AutoBleem-NG/AutoBleem/actions/workflows/ci.yml/badge.svg)](https://github.com/AutoBleem-NG/AutoBleem/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![C++](https://img.shields.io/badge/C%2B%2B-11-00599C?logo=cplusplus)
![Platform](https://img.shields.io/badge/Platform-PlayStation_Classic-003791?logo=playstation)

AutoBleem-NG (Next Generation) is a custom boot menu and game launcher for the PlayStation Classic.

Building on [AKA-Axanar's AutoBleem](https://github.com/AKA-Axanar/AutoBleem) with continued development, bug fixes, and new features.

## Features

- EvolutionUI for browsing and managing games
- Automatic game scanning from USB drives
- BIN/CUE, CHD, and PBP disc image support
- Offline cover art and metadata lookup
- Shared memory cards across games
- RetroArch integration for multi-system emulation
- Multi-disc game support

### AutoBleem-NG Improvements

- 17 languages fully translated with maintainable INI file format
- Docker-based ARM cross-compilation (no toolchain installation required)
- Optimized binary with ARMv8-A/NEON/VFPv4 tuning and UPX compression
- Simplified boot scripts for easier maintenance
- Structured logging (`USB:/System/Logs/autobleem-ng.log`)
- Memory leak and bug fixes
- Modernized codebase with improved safety and expanded test coverage

## Project Structure

```
├── docs/               # Documentation
└── autobleem/
    ├── cmake/          # CMake toolchain files
    ├── code/           # Main source code
    │   ├── engine/     # Game scanning, database, metadata
    │   ├── gui/        # GUI framework and screens
    │   └── launcher/   # Game launcher and emulator integration
    ├── db/             # Cover art databases
    ├── libs/           # Bundled libraries (sqlite, nlohmann/json, libchdr, plog)
    ├── payload/        # PSC payload scripts and structure
    ├── resources/      # Assets (images, fonts, language files)
    ├── scripts/        # Development tools (lang_tools.py)
    └── tests/          # Unit tests
```

## Documentation

- [Building](docs/building.md) - Build instructions and prerequisites
- [Boot Process](docs/boot-process.md) - How AutoBleem boots on the PSC
- [Testing](docs/testing.md) - Running unit tests
- [Translation](docs/translation.md) - Localization guide

## License

GPL-3.0 - See [LICENSE](LICENSE) for details.

## Credits

- [AKA-Axanar/AutoBleem](https://github.com/AKA-Axanar/AutoBleem) - Enhanced AutoBleem this fork builds upon
- [screemerpl/AutoBleem](https://github.com/screemerpl/cbleemsync) - Original AutoBleem project
- See [docs/original-readme.md](docs/original-readme.md#credits-and-links) for full credits
