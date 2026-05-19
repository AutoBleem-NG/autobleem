#!/bin/bash
#
# autobleem.sh - Launch the AutoBleem GUI
#
# Extracts shared libraries to tmpfs for better performance,
# initializes RetroArch/RetroBoot in the background, and
# launches the main EvolutionUI GUI application.
#

# Extract shared libraries to tmpfs
# Running from RAM is faster than USB and avoids wear
mkdir -p /tmp/lib
cp /media/Autobleem/lib/libs.tar.gz /tmp/lib
cd /tmp/lib
tar xvzf libs.tar.gz
/media/Autobleem/bin/autobleem/log_elf.sh /tmp/lib

# Initialize RetroArch/RetroBoot in background (if installed)
[ -f /media/retroarch/retroboot/bin/init.sh ] && sh /media/retroarch/retroboot/bin/init.sh &

# Launch the AutoBleem GUI (EvolutionUI)
cd /media/Autobleem/bin/autobleem
./run.sh
sync
