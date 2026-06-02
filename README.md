# AutoBleem-NG

[![CI](https://github.com/AutoBleem-NG/AutoBleem/actions/workflows/ci.yml/badge.svg)](https://github.com/AutoBleem-NG/AutoBleem/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![C++](https://img.shields.io/badge/C%2B%2B-11-00599C?logo=cplusplus)
![Platform](https://img.shields.io/badge/Platform-PlayStation_Classic-003791?logo=playstation)

AutoBleem-NG (Next Generation) is a maintained AutoBleem UI for the PlayStation
Classic: a boot menu, game scanner, and launcher built around EvolutionUI.

Building on [AKA-Axanar's AutoBleem](https://github.com/AKA-Axanar/AutoBleem) with continued development, bug fixes, and new features.

> **Scope note:** This source repository builds and maintains the AutoBleem-NG
> UI, payload scripts, assets, themes, and support files. A prepared USB
> distribution can also include RetroArch, emulator cores, ROM folders, user
> games, saves, logs, and system backups that are not produced by the UI build
> itself.

## Features

- EvolutionUI for browsing and managing games
- Automatic game scanning from USB drives
- BIN/CUE, CHD, and PBP disc image support
- Offline cover art and metadata lookup
- Shared memory cards across games
- RetroArch integration for multi-system emulation
- Multi-disc game support

## AutoBleem-NG Improvements

- 17 languages fully translated with maintainable INI file format
- Reproducible Docker-based ARM builds with PSC-compatible SDL2 payload libraries
- Optimized binary with ARMv8-A/NEON/VFPv4 tuning and UPX compression
- Simplified boot scripts for easier maintenance
- Structured logging (`USB:/System/Logs/autobleem-ng.log`)
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
    ├── libs/           # Bundled libraries (sqlite, nlohmann/json, libchdr, plog)
    ├── payload/        # PSC payload scripts and structure
    ├── resources/      # Assets (images, fonts, language files)
    ├── scripts/        # Development tools (lang_tools.py)
    └── tests/          # Unit tests
```

## Documentation

- [Bluetooth Controller Setup](docs/bluetooth-setup.md) - Experimental Bluetooth pairing notes
- [Boot Process](docs/boot-process.md) - How AutoBleem boots on the PSC
- [Building](docs/building.md) - Build targets, reproducible ARM builds, and prerequisites
- [Changelog](docs/changelog.md) - Project release history
- [Credits](docs/credits.md) - Project lineage and bundled component credits
- [Distribution Layout](docs/distribution-layout.md) - Expected deployed USB package structure
- [Gamepad Compatibility](docs/gamepad-compatibility.md) - Controller support notes
- [Kernel Installation](docs/kernel-installation.md) - Optional AutoBleem kernel setup
- [Linting](docs/linting.md) - clang-tidy usage
- [Menu Options](docs/menu-options.md) - User-facing menu controls and settings
- [Testing](docs/testing.md) - Running unit tests
- [Translation](docs/translation.md) - Localization guide

## License

GPL-3.0 - See [LICENSE](LICENSE) for details.

## Credits

- [AKA-Axanar/AutoBleem](https://github.com/AKA-Axanar/AutoBleem) - Enhanced AutoBleem this fork builds upon
- [screemerpl/AutoBleem](https://github.com/screemerpl/cbleemsync) - Original AutoBleem project
- See [docs/credits.md](docs/credits.md) for project lineage and bundled component credits
