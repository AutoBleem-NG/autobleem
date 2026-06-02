# AutoBleem Boot Process

This document explains the boot sequence when AutoBleem starts on the PlayStation Classic.

On a deployed USB drive the runtime directory is `Autobleem/` at the USB root.
During boot, the PlayStation Classic mounts the USB root at `/media`, so
`Autobleem/start.sh` is executed as `/media/Autobleem/start.sh`.

## Boot Sequence

```
PSC Power On
     │
     ▼
Sony Exploit → Autobleem/start.sh
     │
     ▼
rc/boot.sh
     ├── bind joystick udev rule and reload udev
     ├── rc/killsony.sh     (kill Sony processes)
     ├── rc/backup.sh       (setup dirs, copy BIOS)
     └── rc/autobleem.sh    (launch GUI)
              │
              ▼
         autobleem-gui
              │
              ├── [Play game] ──► fork/exec launch.sh ──► PCSX ──► return to GUI
              │
              ├── [Play with RetroArch] ──► fork/exec launch_rb.sh ──► RA ──► return to GUI
              │
              ├── [RetroArch menu] ──► exit GUI with AB_SELECTION=4
              │                              │
              │                              ▼
              │                        rc/retroarch.sh ──► RetroArch
              │                              │
              │                              ▼
              │                        ./start.sh (loop back)
              │
              └── [Power button] ──► shutdown -h now
                   or [Exit]
                        │
                        ▼
                   rc/reboot.sh
```

## Key Scripts

| Script | Purpose |
|--------|---------|
| `start.sh` | Entry point, sources boot.sh |
| `boot.sh` | Main orchestrator, applies the USB gamepad udev rule, and checks AB_SELECTION after GUI exits |
| `killsony.sh` | Kills Sony's stock UI processes |
| `backup.sh` | Creates USB dirs, copies BIOS, sets up tmpfs mounts |
| `autobleem.sh` | Extracts libs to RAM, launches GUI |
| `reboot.sh` | Syncs, unmounts, reboots |
| `retroarch.sh` | Launches RetroArch menu, calls start.sh to loop back |
| `launch.sh` | Launches PCSX for PS1 games |
| `launch_rb.sh` | Launches games via RetroArch |

These scripts live in `Autobleem/rc/` on the USB drive, except `start.sh`,
which lives directly under `Autobleem/`.

## Runtime Directories

| USB path | Runtime use |
| --- | --- |
| `Autobleem/bin/autobleem/` | AutoBleem-NG UI binary, config, language files, fonts, music, and UI assets |
| `Autobleem/bin/emu/` | PCSX binary and plugins for the normal PS1 launch path |
| `Autobleem/lib/` | Runtime library archive copied and extracted to RAM before the UI starts |
| `Games/` | PS1 games plus AutoBleem save states and shared memory cards |
| `System/` | BIOS copies, preferences, region data, databases, and logs |
| `retroarch/` | RetroArch runtime used by the RetroArch menu and RetroArch game launch paths |

## GUI Exit Handling

When the GUI exits, `boot.sh` reads `autobleem_cfg.sh` to check what action was selected:

- **AB_SELECTION=4** (RetroArch menu): Run `retroarch.sh`, which launches RetroArch and loops back via `start.sh`
- **Otherwise**: Run `reboot.sh` to reboot the console

Note: Power button triggers `shutdown -h now` directly from C++, bypassing shell scripts.

## Log Files

| Log | Content |
|-----|---------|
| `System/Logs/autobleem-ng.log` | Application log |
| `System/Logs/AB_out.txt` | GUI stdout |
| `System/Logs/AB_err.txt` | GUI stderr |
| `System/Logs/launch.log` | PCSX launch script log |
| `System/Logs/pcsx.log` | PCSX emulator stdout/stderr |
| `System/Logs/readelf.txt` | ELF dependency inspection output |
| `System/Logs/ui_menu.log` | Sony UI menu log copy/placeholder |
