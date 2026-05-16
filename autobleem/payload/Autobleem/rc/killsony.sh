#!/bin/sh
#
# killsony.sh - Terminate Sony's stock UI processes
#
# Forcefully kills Sony's built-in applications to free resources
# and prevent conflicts with AutoBleem's GUI.
#
# Processes killed:
#   - sonyapp: Main Sony application
#   - showLogo: Boot logo display
#   - ui_menu: Sony's game carousel UI
#
# Wait 1s before killing so Sony's processes have finished forking;
# otherwise killall can miss late starters and leak the stock UI.
#

sleep 1
killall -s KILL sonyapp showLogo ui_menu
