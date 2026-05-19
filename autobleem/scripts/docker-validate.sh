#!/bin/sh
# Build-time validation helpers for the Dockerfile.
#
# Usage:
#   docker-validate gcc                    Confirm gcc-6 cross-compiler runs
#   docker-validate sysroot                Confirm sysroot is complete + glibc 2.24
#   docker-validate glibc-symbols <bin>    Assert no GLIBC ref above 2.24
#   docker-validate no-rpath <bin>         Assert no embedded RPATH/RUNPATH
#
# Each subcommand exits non-zero on failure so a `RUN` step aborts the build
# at the point of detection, not 200 lines later in some downstream configure.

set -eu

cmd=${1:-}
shift || true

case "$cmd" in
    gcc)
        test -x /opt/gcc-6/usr/bin/arm-linux-gnueabihf-gcc-6
        test -x /opt/gcc-6/usr/bin/arm-linux-gnueabihf-g++-6
        LD_LIBRARY_PATH=/opt/gcc-6/usr/lib/x86_64-linux-gnu \
            /opt/gcc-6/usr/bin/arm-linux-gnueabihf-gcc-6 --version
        ;;

    sysroot)
        libc=/opt/psc-sysroot/lib/arm-linux-gnueabihf/libc.so.6
        libstdcxx=/opt/psc-sysroot/usr/lib/arm-linux-gnueabihf/libstdc++.so.6
        stdio_h=/opt/psc-sysroot/usr/include/stdio.h
        if ! test -f "$libc" || ! test -f "$libstdcxx" || ! test -f "$stdio_h"; then
            echo "sysroot incomplete" >&2
            ls -la /opt/psc-sysroot/lib/arm-linux-gnueabihf/ >&2 || true
            exit 1
        fi
        # Print the glibc banner for the build log (best-effort; banner format
        # varies across distros).
        grep -aoE "GNU C Library .* version 2\.[0-9]+" "$libc" | head -1 || true
        # Assert the version really is 2.24.
        if ! grep -aqE "GNU C Library .* version 2\.24" "$libc"; then
            echo "Stretch sysroot does not report glibc 2.24" >&2
            exit 1
        fi
        ;;

    glibc-symbols)
        binary=${1:-}
        if [ -z "$binary" ]; then
            echo "usage: $0 glibc-symbols <binary>" >&2
            exit 2
        fi
        max=$(arm-linux-gnueabihf-objdump -T "$binary" \
            | awk '$5 ~ /^GLIBC_/ {sub("GLIBC_","",$5); print $5}' \
            | sort -V | tail -n1)
        if [ -z "$max" ]; then
            echo "OK: $binary has no versioned GLIBC refs"
            exit 0
        fi
        echo "$max" | awk -F. '{
            if ($1 > 2 || ($1 == 2 && $2 > 24)) {
                printf "FAIL: glibc symbol above 2.24 (%s)\n", $0
                exit 1
            }
            printf "OK: max glibc symbol %s\n", $0
        }'
        ;;

    no-rpath)
        binary=${1:-}
        if [ -z "$binary" ]; then
            echo "usage: $0 no-rpath <binary>" >&2
            exit 2
        fi
        if command -v arm-linux-gnueabihf-readelf >/dev/null 2>&1; then
            readelf_bin=arm-linux-gnueabihf-readelf
        else
            readelf_bin=readelf
        fi
        if "$readelf_bin" -d "$binary" | grep -Eq '\((RPATH|RUNPATH)\)'; then
            echo "FAIL: $binary contains embedded RPATH/RUNPATH" >&2
            "$readelf_bin" -d "$binary" | grep -E '\((RPATH|RUNPATH)\)' >&2
            exit 1
        fi
        echo "OK: $binary has no RPATH/RUNPATH"
        ;;

    *)
        echo "usage: $0 {gcc|sysroot|glibc-symbols <binary>|no-rpath <binary>}" >&2
        exit 2
        ;;
esac
