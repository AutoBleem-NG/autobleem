#!/bin/bash
#
# launch_rb.sh - RetroArch/RetroBoot launcher for AutoBleem
#
# Called by EvolutionUI to launch games through RetroArch.
# Kills Sony processes, sets up the environment, and launches
# the game with the specified core.
#
# Arguments:
#   $1 - ROM/image file path
#   $2 - RetroArch core to use
#

# Kill Sony processes and disable power management
killall -s KILL showLogo sonyapp ui_menu auto_dimmer pcsx dimmer
echo 2 > /data/power/disable

echo "Image: $1"
echo "Core: $2"

# Launch game through RetroBoot
sh /media/retroarch/retroboot/bin/launch_rfa_rom.sh "$1" "$2"

# Cleanup - remove load indicator
rm /tmp/.abload
usleep 250000
