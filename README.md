# AutoBleem-NG

AutoBleem-NG (Next Generation) is a custom firmware for the PlayStation Classic, building on [AKA-Axanar's AutoBleem](https://github.com/AKA-Axanar/AutoBleem) improvements.

## Features

- EvolutionUI for browsing and managing games
- Automatic game scanning from USB drives
- BIN/CUE, CHD, and PBP disc image support
- Offline cover art and metadata lookup
- Shared memory cards across games
- RetroArch integration for multi-system emulation
- Multi-disc game support
- Docker-based ARM build (no toolchain installation required)
- 17 languages supported with automated translation tooling
- Structured logging for easier debugging

## Project Structure

```
├── cmake/              # CMake toolchain files
├── docs/               # Documentation
├── libs/               # Bundled libraries (sqlite, nlohmann/json, libchdr, plog)
├── payload/            # PSC payload scripts and structure
├── src/
│   ├── code/           # Main source code
│   │   ├── engine/     # Game scanning, database, metadata
│   │   ├── gui/        # GUI framework and screens
│   │   └── launcher/   # Game launcher and emulator integration
│   ├── resources/      # Assets (images, fonts, language files)
│   └── scripts/        # Development tools (lang_tools.py)
└── tests/              # Unit tests
```

## Logging

Logs are written to `/tmp/autobleem.log` on the PlayStation Classic.

## Documentation

- [Building](docs/building.md) - Build instructions and prerequisites
- [Testing](docs/testing.md) - Running unit tests
- [Translation](docs/translation.md) - Localization guide

## License

GPL-3.0 - See [LICENSE](LICENSE) for details.

## Credits

- [AKA-Axanar/AutoBleem](https://github.com/AKA-Axanar/AutoBleem) - Enhanced AutoBleem this fork builds upon
- [screemerpl/AutoBleem](https://github.com/screemerpl/cbleemsync) - Original AutoBleem project
- See [docs/original-readme.md](docs/original-readme.md#credits-and-links) for full credits
