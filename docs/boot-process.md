# AutoBleem Boot Process

This document explains the boot sequence when AutoBleem starts on the PlayStation Classic.

## Boot Sequence

```
PSC Power On
     │
     ▼
Sony Exploit → Autobleem/start.sh
     │
     ▼
rc/boot.sh
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
| `boot.sh` | Main orchestrator, checks AB_SELECTION after GUI exits |
| `killsony.sh` | Kills Sony's stock UI processes |
| `backup.sh` | Creates USB dirs, copies BIOS, sets up tmpfs mounts |
| `autobleem.sh` | Extracts libs to RAM, launches GUI |
| `reboot.sh` | Syncs, unmounts, reboots |
| `retroarch.sh` | Launches RetroArch menu, calls start.sh to loop back |
| `launch.sh` | Launches PCSX for PS1 games |
| `launch_rb.sh` | Launches games via RetroArch |

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
