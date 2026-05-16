#!/bin/sh
#
# retroarch.sh - Launch RetroArch directly
#
# Starts RetroArch without a specific ROM, allowing the user
# to browse and select content from within RetroArch's menu.
# Returns to AutoBleem UI when RetroArch exits.
#

# Kill Sony processes and disable power management
killall -s KILL showLogo sonyapp ui_menu auto_dimmer pcsx dimmer
echo 2 > /data/power/disable

# Launch RetroArch menu
sh /media/retroarch/retroboot/bin/launch_rfa.sh

# Return to AutoBleem UI
cd /media/Autobleem/
rm /tmp/.abload

# Restart AutoBleem
./start.sh
