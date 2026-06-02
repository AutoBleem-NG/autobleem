# Credits

AutoBleem-NG builds on several earlier PlayStation Classic projects and bundled
open-source components.

## Project Lineage

- [screemerpl/AutoBleem](https://github.com/screemerpl/cbleemsync) - original AutoBleem project and EvolutionUI foundation.
- [AKA-Axanar/AutoBleem](https://github.com/AKA-Axanar/AutoBleem) - enhanced AutoBleem fork this project builds on.
- BleemSync / ModMyClassic contributors - early PlayStation Classic boot and payload scripting work that informed AutoBleem's shell payload flow.

## Payload And Integration

- Screemer and madmonkey - ABFlashKit kernel flasher tooling.
- genderbent - historical RetroArch integration for AutoBleem and the PlayStation Classic.

## Bundled Components

- SQLite - public-domain database engine used for game metadata and play history.
- UNECM by Neill Corlett - ECM decoder used by the scanner.
- nlohmann/json - JSON parsing support.
- libchdr and bundled dependencies - CHD disc image support.
- plog - application logging.

See [autobleem/libs/README.md](../autobleem/libs/README.md) and the license
files under `autobleem/libs/` for bundled library versions and license details.
