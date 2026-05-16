#!/bin/bash
# Append a binary's ELF header and ARM attributes to /media/System/Logs/readelf.txt.
# No-ops if the binary is missing or has already been recorded, so the log
# doesn't grow across launches.
#
# Usage: log_elf.sh <binary-path> [label]

BINARY="$1"
[ -n "$BINARY" ] && [ -f "$BINARY" ] || exit 0

LABEL="${2:-$(basename "$BINARY")}"
LOG=/media/System/Logs/readelf.txt
mkdir -p "$(dirname "$LOG")"

grep -qF "=== $LABEL ===" "$LOG" 2>/dev/null && exit 0

READELF=/media/Autobleem/bin/autobleem/readelf
{
	echo "=== $LABEL ==="
	"$READELF" -h "$BINARY" 2>&1
	"$READELF" -A "$BINARY" 2>&1
} >> "$LOG"
