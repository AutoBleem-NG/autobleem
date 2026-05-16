#!/bin/bash
#
# launch_rom.rfa.sh - RetroArch ROM launcher (RetroBoot integration)
#
# Copied to /media/retroarch/retroboot/bin/ during backup.sh.
# Used by RetroBoot to launch ROMs with specified cores.
#
# Arguments:
#   $1 - ROM/image file path
#   $2 - RetroArch core to use
#

# Kill Sony processes and disable power management
killall -s KILL showLogo sonyapp ui_menu auto_dimmer pcsx dimmer
echo 2 > /data/power/disable

/media/Autobleem/bin/autobleem/log_elf.sh /media/retroarch/bin/retroarch

echo "Image: $1"
echo "Core: $2"

# Launch game through RetroBoot
sh /media/retroarch/retroboot/bin/launch_rfa_rom.sh "$1" "$2"

# Cleanup - remove load indicator
rm /tmp/.abload
usleep 250000
