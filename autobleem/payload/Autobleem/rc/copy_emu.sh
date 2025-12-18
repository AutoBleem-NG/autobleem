#!/bin/sh
#
# copy_emu.sh - Copy Sony's PCSX emulator from internal storage to USB
#
# Copies the original Sony PCSX emulator binary and plugins from
# the PlayStation Classic's internal storage to the USB drive.
# This allows AutoBleem to use the emulator without modifying
# the console's internal storage.
#
# Source locations (PSC internal):
#   /usr/sony/bin/pcsx      - PCSX emulator binary
#   /usr/sony/bin/plugins/  - PCSX plugins
#
# Destination (USB):
#   /media/System/Emulator/pcsx
#   /media/System/Emulator/plugins/
#

EMU_DIR="/media/System/Emulator"

# Create destination directory
mkdir -p "$EMU_DIR/plugins"

# Copy PCSX emulator binary
if [ -f /usr/sony/bin/pcsx ]; then
    echo "Copying PCSX emulator..."
    cp /usr/sony/bin/pcsx "$EMU_DIR/pcsx"
    chmod +x "$EMU_DIR/pcsx"
    echo "  -> $EMU_DIR/pcsx"
else
    echo "ERROR: /usr/sony/bin/pcsx not found"
    exit 1
fi

# Copy PCSX plugins
if [ -d /usr/sony/bin/plugins ]; then
    echo "Copying PCSX plugins..."
    cp -r /usr/sony/bin/plugins/* "$EMU_DIR/plugins/"
    echo "  -> $EMU_DIR/plugins/"
else
    echo "ERROR: /usr/sony/bin/plugins not found"
    exit 1
fi

echo "Done. Sony emulator copied to USB."
sync
