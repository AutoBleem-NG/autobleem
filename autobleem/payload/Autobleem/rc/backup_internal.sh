#!/bin/bash
#
# backup_internal.sh - Backup internal game save data to USB
#
# Copies save states and memory card data from the PSC's internal
# storage to the USB drive for the 20 built-in games.
# Also copies the internal game database if not already present.
#
# Run this to preserve your progress on internal games.
#

# Copy save data for each of the 20 internal games
for STATE in {1..20}
do
  mkdir -p /media/Games/!SaveStates/$STATE
  # Copy save state and memory card data (if not already backed up)
  [ ! -f /media/Games/!SaveStates/$STATE/pcsx.cfg ] && cp -R /data/AppData/sony/pcsx/$STATE/.pcsx/* /media/Games/!SaveStates/$STATE
  [ ! -f /media/Games/!SaveStates/$STATE/pcsx.cfg ] && cp /gaadata/$STATE/pcsx.cfg /media/Games/!SaveStates/$STATE/pcsx.cfg
done

# Copy internal game database (if not already present)
[ ! -f /media/System/Databases/internal.db ] && cp /gaadata/databases/regional.db /media/System/Databases/internal.db

sync
