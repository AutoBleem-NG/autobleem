#!/bin/bash
#
# backup.sh - System setup and file backup
#
# Performs critical system initialization:
#   1. Creates USB directory structure for games, saves, and configs
#   2. Copies BIOS files from internal storage to USB (first run only)
#   3. Copies user preferences and regional settings to USB
#   4. Sets up tmpfs mounts that redirect system paths to USB
#   5. Disables power button to prevent accidental shutdowns
#
# The tmpfs mounts allow Sony's code (which expects /gaadata and /data)
# to transparently read from USB locations instead.
#

# Remount internal storage as read-only for safety
mount -o remount,ro /gaadata

# Create USB directory structure
mkdir -p /media/Games/!SaveStates
mkdir -p /media/System/Bios
mkdir -p /media/System/Preferences
mkdir -p /media/System/Preferences/System
mkdir -p /media/System/Preferences/User
mkdir -p /media/System/Preferences/AutoDimmer
mkdir -p /media/System/Databases
mkdir -p /media/System/Region
mkdir -p /media/System/Logs
mkdir -p /media/System/UI

# Copy AutoBleem RetroArch metadata
cp -r /media/Autobleem/bin/autobleem/AutoBleem.rdb /media/retroarch/database/rdb/

# Copy BIOS files to USB (only if not already present)
[ ! -f /media/System/Bios/romw.bin ] && cp -r /gaadata/system/bios/romw.bin /media/System/Bios/romw.bin
[ ! -f /media/System/Bios/romJP.bin ] && cp -r /media/System/Bios/romw.bin /media/System/Bios/romJP.bin

# Copy regional preferences (UI region settings)
[ ! -f /media/System/Preferences/System/regional.pre ] && cp /gaadata/preferences/* /media/System/Preferences/System

# Copy user preferences (language settings, etc.)
[ ! -f /media/System/Preferences/User/user.pre ] && cp /data/AppData/sony/ui/* /media/System/Preferences/User

# Copy auto-dimmer configuration
[ ! -f /media/System/Preferences/AutoDimmer/config.cnf ] && cp /data/AppData/sony/auto_dimmer/* /media/System/Preferences/AutoDimmer

# Copy region info
[ ! -f /media/System/Region/REGION ] && cp /gaadata/geninfo/* /media/System/Region

# Copy UI error log
[ ! -f /media/System/UI/error.log ] && cp /data/sony/ui/* /media/System/UI

# Initialize UI menu log
[ ! -f /media/System/Logs/ui_menu.log ] && touch /media/System/Logs/ui_menu.log

# Create tmpfs mounts to redirect system paths to USB
# This allows Sony's code to read from USB while expecting /gaadata and /data
mkdir -p /tmp/gaadatatmp /tmp/datatmp

# Set up /gaadata redirection (game data, BIOS, databases)
mkdir -p /tmp/gaadatatmp/system/
ln -s /media/System/Databases /tmp/gaadatatmp/databases
ln -s /media/System/Region /tmp/gaadatatmp/geninfo
ln -s /media/System/Bios /tmp/gaadatatmp/system/bios
ln -s /media/System/Preferences/System /tmp/gaadatatmp/preferences

# Set up /data redirection (user data, save states, preferences)
mkdir -p /tmp/datatmp/sony/sgmo /tmp/datatmp/AppData/sony
ln -s /tmp/diag /tmp/datatmp/sony/sgmo/diag
ln -s /dev/shm/power /tmp/datatmp/power
ln -s /media/System/UI /tmp/datatmp/sony/ui
ln -s /media/System/Preferences/User /tmp/datatmp/AppData/sony/ui
ln -s /media/System/Preferences/AutoDimmer /tmp/datatmp/AppData/sony/auto_dimmer
cp -r /usr/sony/share/recovery/AppData/sony/pcsx /tmp/datatmp/AppData/sony/pcsx
ln -s /media/System/Bios /tmp/datatmp/AppData/sony/pcsx/bios
ln -s /usr/sony/bin/plugins /tmp/datatmp/AppData/sony/pcsx/plugins

# Disable power button to prevent accidental shutdowns during use
echo 2 > /tmp/datatmp/power/disable
