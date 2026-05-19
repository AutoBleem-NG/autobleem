#!/bin/bash
# Append ELF headers and ARM attributes to /media/System/Logs/readelf.txt.
# Accepts either a single file or a directory tree. Directory scans detect ELF
# files by content, so new libraries/plugins are picked up automatically.
#
# Usage: log_elf.sh <file-or-dir> [label]

TARGET="$1"
[ -n "$TARGET" ] || exit 0

LABEL_OVERRIDE="$2"
LOG=/media/System/Logs/readelf.txt
READELF=/media/Autobleem/bin/autobleem/readelf

[ -x "$READELF" ] || exit 0
mkdir -p "$(dirname "$LOG")"

is_elf() {
	local file="$1"
	[ -f "$file" ] || return 1
	[ "$(dd if="$file" bs=4 count=1 2>/dev/null | od -An -tx1 | tr -d ' \n')" = "7f454c46" ]
}

dump_elf() {
	local file="$1"
	local label="$2"

	grep -qF "=== $label ===" "$LOG" 2>/dev/null && return 0

	{
		echo "=== $label ==="
		"$READELF" -h "$file" 2>&1
		"$READELF" -A "$file" 2>&1
	} >> "$LOG"
}

scan_dir() {
	local root="$1"
	local label_root="$2"
	local file rel label

	find -L "$root" -type f | sort | while read -r file; do
		is_elf "$file" || continue
		rel="${file#$root/}"
		label="${label_root%/}/$rel"
		dump_elf "$file" "$label"
	done
}

if [ -d "$TARGET" ]; then
	scan_dir "$TARGET" "${LABEL_OVERRIDE:-$TARGET}"
elif is_elf "$TARGET"; then
	dump_elf "$TARGET" "${LABEL_OVERRIDE:-$TARGET}"
fi
