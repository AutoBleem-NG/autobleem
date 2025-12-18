#!/bin/sh
#
# boot.sh - Main boot orchestrator for AutoBleem
#
# Called by start.sh after the PSC exploit loads AutoBleem.
# Coordinates the entire boot sequence:
#   1. Fix USB gamepad detection for dual controllers
#   2. Kill Sony's stock UI processes
#   3. Set up directory structure and copy system files
#   4. Launch the AutoBleem GUI
#   5. Launch RetroArch if selected, otherwise reboot
#

# USB gamepad fix - allows both controllers through USB hub
mount -o bind /media/Autobleem/rc/20-joystick.rules /etc/udev/rules.d/20-joystick.rules
udevadm control --reload-rules
udevadm trigger

# Execute boot sequence
./killsony.sh
./backup.sh
./autobleem.sh

# Check if user selected RetroArch (MENU_OPTION_RETRO = 4)
. ./autobleem_cfg.sh
if [ "$AB_SELECTION" = "4" ]; then
	./retroarch.sh
else
	./reboot.sh
fi
