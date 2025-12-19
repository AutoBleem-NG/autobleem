# RetroBoot for AutoBleem

Launch RetroArch or EmulationStation from AutoBleem.

## FAQ

### Adding Games to RetroArch

From the RetroArch home screen, scroll to the plus sign icon. Select it and choose the file or folder you wish to scan into the menus.

### Games Not Showing in Playlists

The automatic scanner requires games to be in the libretro database. Recognized romsets are listed at: https://github.com/libretro/libretro-database

**Manual Playlist Editing:**
- Playlists stored in `/retroarch/playlists/`
- Edit instructions: https://docs.libretro.com/guides/roms-playlists-thumbnails/
- Web-based editor: https://www.marcrobledo.com/retroarch-playlist-editor/

When editing, specify the path on your PSC - the USB stick appears as `/media/` so default roms folder is `/media/roms/`.

RetroArch 1.8.4+ includes a manual scanner that doesn't require database matching.

### Setting Up Thumbnails

Follow instructions at: https://docs.libretro.com/guides/roms-playlists-thumbnails/

- Thumbnails folder: `/retroarch/thumbnails/`
- Official thumbnails: https://github.com/libretro/libretro-thumbnails

### Installing Eris Mods

Create `/RB_PATCH` folder on USB root and place `.mod` files inside. Compatible mods install automatically on next RetroBoot start.

### Playlist Goes Blank When Scanning

When scanning large game collections, RetroArch may temporarily blank the playlist while working. Wait for completion.

### Formatting Large Drives as FAT32

Windows default utility won't format large drives as FAT32. Use guiformat by Ridgecrop Consultants: http://www.ridgecrop.demon.co.uk/index.htm?guiformat.htm

Recommended: Set "Allocation unit size" to 32768.

### PSC Boots to Regular Menus

Your USB drive wasn't detected. Check:
- Drive formatted as FAT32 with volume label `SONY` (all capitals)
- All files copied from archive
- PSC has limited USB power - use powered hub for hard drives
- Some USB 3 sticks are incompatible - try USB 2.0

### PS1 BIOS Setup

Not required. RetroBoot automatically copies BIOS from PSC internal memory on first run.

### RetroArch Help

Full documentation: https://docs.libretro.com/

### RetroBoot Configuration

Config file: `/retroarch/retroboot/retroboot.cfg`

### EmulationStation ROM Folders

| System | Path |
|--------|------|
| 3DO | `/roms/3do` |
| Amstrad CPC | `/roms/amstradcpc` |
| Arcade (Daphne) | `/roms/daphne` |
| Arcade (FBA 2012) | `/roms/fba2012` |
| Arcade (FBNeo) | `/roms/arcade` |
| Arcade (MAME) | `/roms/mame` |
| Atari 2600 | `/roms/a2600` |
| Atari 5200 | `/roms/a5200` |
| Atari 7800 | `/roms/a7800` |
| Atari 800 | `/roms/atari800` |
| Atari Jaguar | `/roms/atarijaguar` |
| Atari Lynx | `/roms/atarilynx` |
| Atari ST | `/roms/atarist` |
| Bandai WonderSwan | `/roms/wonderswan` |
| Colecovision | `/roms/colecovision` |
| Commodore 64 | `/roms/c64` |
| Commodore Amiga | `/roms/amiga` |
| IBM PC (DOSBox) | `/roms/dosbox` |
| Magnavox Odyssey2 | `/roms/odyssey2` |
| Mattel Intellivision | `/roms/intellivision` |
| Milton Bradley Vectrex | `/roms/vectrex` |
| MSX | `/roms/msx` |
| NEC PC Engine | `/roms/pcengine` |
| NEC PC Engine CD | `/roms/pce-cd` |
| NEC SuperGrafx | `/roms/supergrafx` |
| NEC TurboGrafx 16 | `/roms/tg16` |
| NEC TurboGrafx CD | `/roms/tg-cd` |
| Nintendo 64 | `/roms/n64` |
| Nintendo DS | `/roms/nds` |
| Nintendo Entertainment System | `/roms/nes` |
| Nintendo Entertainment System Hacks | `/roms/nesh` |
| Nintendo Famicom | `/roms/famicom` |
| Nintendo Famicom Disk System | `/roms/fds` |
| Nintendo Game and Watch | `/roms/gameandwatch` |
| Nintendo Game Boy | `/roms/gb` |
| Nintendo Game Boy Advance | `/roms/gba` |
| Nintendo Game Boy Advance Hacks | `/roms/gbah` |
| Nintendo Game Boy Color | `/roms/gbc` |
| Nintendo Game Boy Hacks | `/roms/gbh` |
| Nintendo Super Famicom | `/roms/sfc` |
| Nintendo Virtual Boy | `/roms/virtualboy` |
| ScummVM | `/roms/scummvm` |
| Sega 32X | `/roms/sega32x` |
| Sega CD | `/roms/segacd` |
| Sega Dreamcast | `/roms/dreamcast` |
| Sega Game Gear | `/roms/gamegear` |
| Sega Game Gear Hacks | `/roms/ggh` |
| Sega Genesis | `/roms/genesis` |
| Sega Genesis Hacks | `/roms/genesish` |
| Sega Master System | `/roms/mastersystem` |
| Sega Mega Drive | `/roms/megadrive` |
| Sega NAOMI / Atomiswave | `/roms/naomi` |
| Sega Saturn | `/roms/saturn` |
| Sega SG-1000 | `/roms/sg1000` |
| Sinclair ZX Spectrum | `/roms/zxspectrum` |
| SNK Neo Geo | `/roms/neogeo` |
| SNK Neo Geo CD | `/roms/neogeocd` |
| SNK Neo Geo Pocket | `/roms/ngp` |
| SNK Neo Geo Pocket Color | `/roms/ngpc` |
| Sony PlayStation | `/roms/psx` |
| Sony PlayStation Portable | `/roms/psp` |
| Super Nintendo Entertainment System | `/roms/snes` |
| Super Nintendo Entertainment System Hacks | `/roms/snesh` |
| Videos | `/roms/videos` |

### Safety and Risks

RetroBoot runs from USB with no system file modification. Built-in games accessed in read-only mode.

RetroBoot has minimal risk of bricking your system, but is offered with no warranty.

## Changelog

### v1.1.0 (04/26/2020)
- Updated to RetroArch 1.8.5
- Added EmulationStation
- Added .mod compatibility mode
- Removed RNDIS support
- Improved updater experience
- Updated cores, configurations, assets, databases

### v1.0.1 (03/05/2020)
- Fixed application playlist issues

### v1.0 (03/03/2020)
- Updated to RetroArch 1.8.4
- Added app launcher system
- Added experimental RNDIS support
- Switched to Ozone as default skin
- Updated cores, mappings, configurations, assets, databases

### v0.10.1 (28/11/2019)
- Fixed core options saving issues
- Fixed Mednafen PCE Turbo Fire

### v0.10 (26/11/2019)
- Updated to RetroArch 1.8.1
- Set fastboot mode as default
- Improved updater performance
- Updated cores, configurations, assets, databases, shader pack

### v0.9 (19/06/2019)
- Boot script rewrite
- Added config file at `/retroarch/retroboot/retroboot.cfg`
- Added patch installer, DS4 mapping handler, fastboot mode
- Merged PSC RetroBoot and RetroBoot for AutoBleem

### v0.8b (06/05/2019)
- Fixed video settings crash
- Fixed threaded video
- Added GLCore backend

### v0.8 (05/05/2019)
- Updated to RetroArch 1.7.7
- Updated cores to KMFDManic custom release 4-29-19
- Moved cheats to separate add-on

### v0.7.5b (21/04/2019)
- Fixed shut-down issues

### v0.7.5 (21/04/2019)
- Updated cores to KMFDManic custom release 4-15-19
- Added shut down message

### v0.7.1 (03/04/2019)
- Updated RetroArch build (1.7.6c RetroBoot)
- Fixed shaders, improved stability

### v0.7 (01/04/2019)
- New RetroArch build (1.7.6b RetroBoot)
- LZMA core compression support
- Updated cores to KMFDManic custom release 4-1-19

### v0.6 (17/03/2019) - Withdrawn
- Updated to RetroArch 1.7.6
- Added cheats
- Updated cores to KMFDManic release 3-16-19

### v0.5.5 (11/03/2019) - Withdrawn
- Added xinput controller support
- Improved logging, payload cleanup

### v0.5 (10/03/2019) - Withdrawn
- New payload launcher
- Migrated to KMFDManic's Cores

### v0.4.1 (26/01/2019)
- Fixed Mupen64Plus core

### v0.4 (26/01/2019)
- Parallelized boot sequence
- Log monitoring for core initialization errors
- Ozone and glui menu driver assets
- Fixed shader presets
- Restructured file layout

### v0.3.1 (22/01/2019)
- Updated NES cores

### v0.3 (22/01/2019)
- Memory card importing from internal storage
- Crash recovery for RetroArch
- Automatic restoration of default PS1 playlist on delete
- Overhauled boot sequence with LED indicators
- Mapped case buttons
- Updated RetroArch binary (thanks CompCom)
- Included common-shaders from RetroPie
- Increased autosave interval to 30 seconds

### v0.2 (18/01/2019)
- Virtual tray support for built-in multi-disc games (MGS & FF7)
- Added PPSSPP core for PSP support
- Enabled SRAM Autosave (5 second intervals)
- Enabled PSX BIOS boot logo
- Switched to XMB menu color 0 (Legacy Red)

### v0.1 (17/01/2019)
- Initial release

## Credits

- Assembled by /u/genderbent for /r/PlaystationClassic
- Cores provided by KMFDManic (https://github.com/KMFDManic/NESC-SNESC-Modifications/releases)
- soramloader thanks to madmonkey1907
- Special thanks to CompCom

## License

GPL-3.0 - See [LICENSE](../LICENSE) for full text.

Core sources available at https://github.com/KMFDManic/
Contact psc.retroboot@gmail.com for additional sources.
