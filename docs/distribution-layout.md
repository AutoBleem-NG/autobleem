# USB Distribution Layout

This document describes the deployed AutoBleem-NG USB layout. It is based on
the package layout observed under `/mnt/usb`.

The source repository builds and maintains the AutoBleem-NG UI, payload scripts,
assets, themes, and bundled support files. A prepared USB distribution can also
include RetroArch, emulator cores, ROM folders, user games, saves, logs, and
system backups that are not produced by the UI build itself.

## Top-Level Layout

| Path | Purpose |
| --- | --- |
| `Autobleem/` | Main AutoBleem runtime. Contains boot scripts, the UI binary and assets, PCSX, and runtime libraries. |
| `Apps/` | Apps shown in the Apps set, including Kernel Installer (ABFlashKit) when present. |
| `Games/` | User PS1 games, AutoBleem save states, and shared memory cards. |
| `System/` | BIOS backups, database copies, preferences, region data, and logs created or updated at runtime. |
| `retroarch/` | RetroArch runtime, cores, assets, configs, playlists, saves, states, and thumbnails. |
| `roms/` | RetroArch ROM folders by system. |
| `themes/` | AutoBleem UI themes and theme assets. |
| `028c18a9-ec4b-4632-b2cf-d4e20f252e8f/` | PlayStation Classic exploit payload directory. |

`System Volume Information/` can appear when the USB drive has been mounted on
Windows. It is not part of AutoBleem-NG.

## AutoBleem Runtime

The deployed AutoBleem runtime lives under `Autobleem/`:

| Path | Purpose |
| --- | --- |
| `Autobleem/start.sh` | Entry point called by the PlayStation Classic exploit. |
| `Autobleem/rc/` | Boot, launch, reboot, RetroArch handoff, backup, and device setup scripts. |
| `Autobleem/bin/autobleem/` | AutoBleem-NG UI binary, configuration, language files, fonts, music, and UI assets. |
| `Autobleem/bin/emu/` | PCSX emulator binary and plugins used by the normal PS1 launch path. |
| `Autobleem/lib/libs.tar.gz` | Runtime library archive extracted to RAM before the UI starts. |

The ARM build output maps into this runtime area when preparing a package:

| Build output | Deployed destination |
| --- | --- |
| `build_arm/autobleem-gui` | `Autobleem/bin/autobleem/autobleem-gui` |
| `build_arm/readelf` | `Autobleem/bin/autobleem/readelf` |
| `build_arm/libs.tar.gz` | `Autobleem/lib/libs.tar.gz` |
| `build_arm/` UI assets and config files | `Autobleem/bin/autobleem/` |

## Runtime Data

AutoBleem creates or updates these directories while running on the console:

| Path | Purpose |
| --- | --- |
| `Games/!SaveStates/` | Per-game PCSX save states, memory cards, screenshots, cheats, patches, and config copies. |
| `Games/!MemCards/` | Shared custom memory cards. |
| `System/Bios/` | BIOS backups copied from the console on first boot. |
| `System/Databases/` | Console and regional database copies. |
| `System/Preferences/` | System, user, and auto-dimmer preference backups. |
| `System/Region/` | Region data copied from the console. |
| `System/Logs/` | AutoBleem, launch, PCSX, RetroArch handoff, and ELF inspection logs. |
| `System/UI/` | Sony UI error log copy. |

Games and save-state folders seen on a working USB drive are user data. They
should not be treated as required files for a clean release package.

## Release Sanity Checks

Before distributing or testing a prepared USB package, verify:

1. The root contains `Autobleem/`, `Apps/`, `Games/`, `System/`, `retroarch/`,
   `roms/`, and `themes/` when the package is intended to include the full
   runtime bundle.
2. `Autobleem/bin/autobleem/` contains the current `autobleem-gui`, languages,
   fonts, default config, PCSX config template, and UI assets.
3. `Autobleem/lib/libs.tar.gz` matches the current ARM build.
4. `Autobleem/bin/emu/pcsx-ab` and its plugins are present for the normal PS1
   launch path.
5. `retroarch/` contains the RetroArch binary, cores, assets, configs, and
   playlists needed by the RetroArch launch paths.
6. Kernel Installer (ABFlashKit) is present in the Apps set if the package
   documents kernel installation.
7. `themes/default/` contains the default theme assets, including `theme.ini`,
   button icons, background image, fonts, and music.
8. A clean boot creates or updates `System/Logs/` without missing-directory
   errors.
