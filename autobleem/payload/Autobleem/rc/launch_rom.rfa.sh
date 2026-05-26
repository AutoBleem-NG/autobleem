#!/bin/bash
#
# launch_rom.rfa.sh - Legacy RetroArch ROM launcher
#
# Kept for compatibility with older payloads that still call this script.
#
# Arguments:
#   $1 - ROM/image file path
#   $2 - RetroArch core to use
#

# Kill Sony processes and disable power management
killall -s KILL showLogo sonyapp ui_menu auto_dimmer pcsx dimmer
echo 2 > /data/power/disable

/media/Autobleem/bin/autobleem/log_elf.sh /media/retroarch/bin/retroarch
/media/Autobleem/bin/autobleem/log_elf.sh /media/retroarch/bin
/media/Autobleem/bin/autobleem/log_elf.sh /media/retroarch/cores

echo "Image: $1"
echo "Core: $2"

# Launch game through RetroArch
sh /media/retroarch/retroboot/bin/launch_rfa_rom.sh "$1" "$2"

# Cleanup - remove load indicator
rm /tmp/.abload
usleep 250000
