#!/bin/sh
#
# reboot.sh - Cleanup and reboot after GUI exits
#
# Called when the user exits the AutoBleem GUI.
# Syncs filesystems to ensure all data is written to USB,
# unmounts the USB drive safely, and reboots the console.
#

sync
umount /media
sync
reboot
