# Legacy Release Notes

This document contains release notes from the original AutoBleem project (versions 0.8.0 through 1.0.0).

For current AutoBleem-NG changes, see [CHANGELOG.md](../CHANGELOG.md).

---

## Version 1.0.0

1. **Better game controller support** - Game controllers in `gamecontrollerdb.txt` should be automatically detected.

2. **CHD game file support** - Support for CHD (Compressed Hunks of Data) format.

3. **Lightgun game support** - Set a game as a lightgun game in the Edit Game Parameters menu. Features include:
   - Edit Game Parameters menu for RetroArch games
   - Select screen for all lightgun-marked games
   - Gun4IR and AE lightguns work on the PSC
   - PS1 games must use RetroArch (pcsx cannot handle lightgun games)
   - Setting a game as lightgun automatically sets "play with RA"
   - Note: Sinden lightguns will not work (requires app to run on system)

4. Bug fixes

---

## Version 0.9.0

### AutoBleem Kernel

AutoBleem now has its own Linux kernel for the PlayStation Classic.

**Kernel Features:**
- OTG support
- RNDIS support
- Easy WiFi setup with auto-connect on boot (all Linux drivers)
- Ethernet support (all Linux drivers)
- NTFS and exFAT filesystem support
- Experimental Bluetooth support

**Flashing the Kernel:**
The kernel flasher will first backup your existing PSC kernel, then flash the new AutoBleem kernel. If you have BleemSync 1.0/1.1/1.2 or Project Eris installed, you must first restore your PSC to original state. See [kernel-installation.md](kernel-installation.md) for instructions.

### WiFi

Easy setup via Network App or Advanced menu (L1 + Square). AutoBleem scans for nearby SSIDs. After network initialization, IP addresses are displayed. Set your timezone and AutoBleem will sync the date/time. WiFi auto-reconnects on every boot. See [wifi-setup.md](wifi-setup.md) for details.

### Date and Time

If WiFi or Ethernet is setup, AutoBleem automatically sets the current date and time from the internet. The last time a game was played is displayed on the PS1 Game carousel. To change the date/time format, edit `Datetimeformat` in `Autobleem/bin/autobleem/config.ini`. See http://www.cplusplus.com/reference/ctime/strftime/ for format strings.

> **Tip:** Press L1 + Square at the AutoBleem start screen to access the Hardware Information App, which displays WiFi/Ethernet IP addresses and current date/time (requires AutoBleem Kernel).

### Bluetooth

Experimental in this release. The Switch Pro controller works. See [bluetooth-setup.md](bluetooth-setup.md) for pairing instructions.

### RetroBoot and Emulation Station

RetroBoot 1.1 is built into AutoBleem and includes Emulation Station. Start ES from the applications folder in RetroArch. Switch back via the ports section in ES.

### Included Emulators

- **AmiBerry** - Amiga Emulator
- **OpenBOR** - Beats of Rage Emulator

### Included Game Ports

- Doom
- Wolfenstein 3D
- Tyrian
- Prince of Persia
- Duke Nukem 3D

---

## Version 0.8.7

### RetroBoot 1.0.1

RetroBoot 1.0.1 is integrated into AutoBleem 0.8.7.

### Apps

Wolfenstein 3D, Tyrian, Prince of Persia, and Duke Nukem 3D added to Apps directory.

### Controller Configuration

- When configuring a new controller and you make a mistake, press Circle to try again
- No need to know if DPAD is analog or digital - just press any DPAD button

### Image Fallback

If no image found in Named_Boxarts for a RetroArch game, AutoBleem looks in Named_Titles, then Named_Snaps.

### Carousel Navigation

Fast forward through game first letters by holding L1 or R1.

### PS1 Game History

New PS1 Game History selection in the game directory menu (similar to RetroArch game history).

### Keyboard Editor

- Cursor now movable within the string (hold L2 to move cursor)
- USB keyboard support for direct text entry
- Keys: Backspace, Delete, Arrow keys, Enter (=X), Escape (=Circle), Tab (return to controller)

### Menu Navigation

- Hold joystick direction to fast forward through selections
- L1/R1: Move one page in vertical menus, multiple items in horizontal menus
- L2/R2: Move to top/bottom or first/last item
- USB keyboard support: cursor keys, home, end, page up/down, Enter, Escape

### Random Selection

- **Start** in carousel: Random game
- **Start** in theme list: Random theme
- **Start** in music list: Random music track

---

## Version 0.8.5

### Applications

Press Select until you get to Apps. Four applications included:
- Amiberry (Amiga emulator)
- Doom Shareware
- OpenBOR (Open Beats of Rage)
- RetroBoot

### Doom

Add additional WAD files to `/Apps/doom` on your USB stick.

### OpenBOR

Copy OpenBOR PAK files to `\Apps\openbor\OpenBOR\Paks`. One PAK auto-loads; multiple PAKs show a selection list.

### Amiberry

For advanced users. Requires BIOS.

### Select Screen Changes

Reduced to three states: PS1 Games, RetroArch games, Applications.

- **L2 + Select** on PS1 carousel: Category menu (All Games, Internal Games, USB Game Directory, Favorites)
- If "Show Internal Games" is off: Categories are Game Directory and Favorites
- Favorites selection at end of menu (like RetroArch playlists)

### RetroArch History

RetroArch playlist menu now includes History playlist. Favorites and History at end of playlist menu.

---

## Version 0.8.1

Fixes two problems in 0.8.0:

1. Fixed crash when starting EvoUI with empty `/Games` directory
2. Fixed always forcing rescan when games failed verify step
3. Splash message displayed when game fails verify (reasons output to `gamesThatFailedVerifyCheck.txt`)
4. Bug fixes

---

## Version 0.8.0

### USB Games Sub-directories

Games can be organized in a directory hierarchy under `/Games`:
```
/Games/Sports/Baseball
/Games/Sports/Football
/Games/Sports/Soccer
```

Select sub-directory with L2 + Select while viewing the carousel. Selecting a directory shows all games in that directory plus sub-directories.

Duplicate games across directories show only the one in the highest-level directory.

**Generated Files** (in `Autobleem/bin/autobleem/`):
- `gameHierarchy_beforeScan.txt` - Games list before removing invalid/duplicates
- `gameHierarchy_afterScanAndRemovingDuplicates.txt` - Final carousel list
- `duplicateGames.txt` - List of duplicate games
- `gamesThatFailedVerifyCheck.txt` - Invalid games removed

### Consolidated Themes

Theme data now contained in single directory under `/Themes` (no longer split between `/Autobleem/bin/autobleem/theme` and `/Themes`). Drag and drop themes to `/Themes`.

### Deleting Games

Delete games from within Game Manager menu. If last copy, prompted to delete SaveStates.

### Internal Game Favorites

Internal games can be set as favorites (data in `/System/Databases/internal.db`).

### RetroArch Favorites

Listed as RA playlist at bottom of playlist menu. Set favorites in RetroArch, not AutoBleem UI.

### RetroBoot 0.10.1

Pre-installed in AutoBleem 0.8.0.
