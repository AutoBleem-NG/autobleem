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

killall -s KILL sonyapp showLogo ui_menu
