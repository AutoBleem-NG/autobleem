#!/bin/bash
#
# launch.sh - PCSX emulator launcher for AutoBleem
#
# Called by EvolutionUI when the user selects a PS1 game.
# Sets up the PCSX environment and launches the emulator.
#
# Arguments:
#   $1 - Save state folder path
#   $2 - Game file path (.cue, .pbp, .chd)
#   $3 - Language code
#   $4 - Region code
#   $5 - Game folder path
#   $6 - Resume point (0 = none, otherwise save state slot)
#   $7 - Aspect ratio (0 = 4:3, 1 = stretched)
#   $8 - Filter setting (0 = off, 1 = bilinear)
#
# Log files:
#   /media/System/Logs/launch.log - This script's log
#   /media/System/Logs/pcsx.log   - PCSX emulator output
#

# Setup logging
LOGDIR="/media/System/Logs"
LOGFILE="$LOGDIR/launch.log"
PCSX_LOG="$LOGDIR/pcsx.log"
mkdir -p "$LOGDIR"

echo "=== launch.sh started at $(date) ===" >> "$LOGFILE"
echo "Arguments: $@" >> "$LOGFILE"

# Copy game-specific PCSX configuration
# Internal games (from /gaadata) use default config
if [[ $5 == *"/gaadata"* ]]; then
  echo "Internal game - using default config" >> "$LOGFILE"
else
  echo "Copying pcsx.cfg from $5 to $1" >> "$LOGFILE"
  cp "$5/pcsx.cfg" "$1/pcsx.cfg"
fi

# Copy custom PCSX binary to /tmp for execution
echo "Using custom PCSX: /media/Autobleem/bin/emu/pcsx-ab" >> "$LOGFILE"
cp -f /media/Autobleem/bin/emu/pcsx-ab /tmp/pcsx
[ -f /tmp/pcsx ] && chmod +x /tmp/pcsx

echo "Starting PCSX with game: $2" >> "$LOGFILE"

# Create clean PCSX runtime directory
rm -rf /tmp/runpcsx
mkdir -p /tmp/runpcsx
cd /tmp/runpcsx

# Set up symlinks for PCSX to find its resources
ln -s "$1" /tmp/runpcsx/.pcsx           # Save states and memory cards
ln -s /media/System/Bios /tmp/runpcsx/bios
ln -s /media/Autobleem/bin/emu/plugins /tmp/runpcsx/plugins

# Launch PCSX with appropriate arguments
if [ "$6" == "0" ]; then
  # Fresh start (no save state)
  echo "PCSX cmd: /tmp/pcsx -filter $8 -ratio $7 -lang $3 -region 4 -enter 1 -cdfile \"$2\"" >> "$LOGFILE"
  /tmp/pcsx -filter $8 -ratio $7 -lang $3 -region 4 -enter 1 -cdfile "$2" > "$PCSX_LOG" 2>&1
  EXITCODE=$?
else
  # Resume from save state
  echo "PCSX cmd: /tmp/pcsx -filter $8 -ratio $7 -lang $3 -region 4 -enter 1 -load $6 -cdfile \"$2\"" >> "$LOGFILE"
  /tmp/pcsx -filter $8 -ratio $7 -lang $3 -region 4 -enter 1 -load $6 -cdfile "$2" > "$PCSX_LOG" 2>&1
  EXITCODE=$?
fi

# Log exit status
echo "PCSX exited with code: $EXITCODE" >> "$LOGFILE"
if [ $EXITCODE -ne 0 ]; then
  echo "ERROR: PCSX crashed or failed! Check $PCSX_LOG for details" >> "$LOGFILE"
fi
echo "=== launch.sh finished at $(date) ===" >> "$LOGFILE"
