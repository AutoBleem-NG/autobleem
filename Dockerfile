# AutoBleem ARM build environment — modern host + Debian Stretch sysroot.
#
# Host: Ubuntu 24.04. Sysroot: Debian 9 "Stretch" armhf at /opt/psc-sysroot
# (glibc 2.24 / libstdc++ 6.0.22 — matches PSC firmware). Cross compiler:
# Stretch gcc-6 extracted to /opt/gcc-6, the newest gcc whose libstdc++ ABI
# matches glibc 2.24. SDL2 family + libchdr + autobleem-gui all built with
# --sysroot=/opt/psc-sysroot.
#
# Build:   make build
# Extract: make extract
FROM ubuntu:24.04

LABEL maintainer="AutoBleem Team"
LABEL description="Modern-toolchain build env using a Debian Stretch armhf sysroot"

# Version info from the host (set by Makefile from .git).
ARG GIT_BRANCH=unknown
ARG GIT_CHANGED=false
ARG GIT_COMMIT_HASH=unknown
ARG GIT_VERSION=1.1.0-dev

ARG BINUTILS_VERSION=2.42
ARG ENABLE_UPX=false
ARG UPX_VERSION=5.1.1

# SDL2 2.0.12 matches libSDL2-2.0.so.0.12.0 from the original PSC payload.
# Newer builds can init cleanly yet still leave the PSC at a black screen.
# SDL2_mixer 2.6.x is the first line where Mix_LoadWAV is a real exported
# symbol (autobleem-gui calls it directly) and the last line ABI-compatible
# with SDL2 2.0.12 — 2.8.x requires SDL2 2.0.24+.
ARG SDL2_VERSION=2.0.12
ARG SDL2_IMAGE_VERSION=2.6.3
ARG SDL2_MIXER_VERSION=2.6.3
ARG SDL2_TTF_VERSION=2.20.2

ENV DEBIAN_FRONTEND=noninteractive

# Host tooling. Deliberately NOT installing Ubuntu's gcc-arm-linux-gnueabihf:
# it ships a libstdc++ that needs glibc 2.34+ (merged pthread, 64-bit time_t,
# getentropy, ...) — won't run on PSC's glibc 2.24. gcc-6 comes from Stretch
# in a later stage; only binutils + small host deps live here.
RUN apt-get update && apt-get install -y --no-install-recommends \
        bash ca-certificates cmake git gnupg make pkg-config wget xz-utils \
        g++ gcc libexpat1-dev libffi-dev \
        binutils-arm-linux-gnueabihf \
        debian-archive-keyring mmdebstrap \
    && rm -rf /var/lib/apt/lists/*

# Validation helpers used throughout the build. Copied early so every stage
# below can shell out instead of inlining test logic.
COPY autobleem/scripts/docker-validate.sh /usr/local/bin/docker-validate

# UPX for compressing the final autobleem-gui binary.
RUN if [ "${ENABLE_UPX}" = "true" ]; then \
        wget -q https://github.com/upx/upx/releases/download/v${UPX_VERSION}/upx-${UPX_VERSION}-amd64_linux.tar.xz && \
        tar -xf upx-${UPX_VERSION}-amd64_linux.tar.xz && \
        cp upx-${UPX_VERSION}-amd64_linux/upx /usr/local/bin/ && \
        chmod +x /usr/local/bin/upx && \
        rm -rf upx-${UPX_VERSION}-amd64_linux*; \
    else \
        echo "UPX disabled"; \
    fi

# Build the PSC-matched sysroot at /opt/psc-sysroot.
#
# `--variant=extract` unpacks .debs without running maintainer scripts (we
# just need the files). Stretch's Release file is expired so we disable
# validity checks; the expired signing key still lives in debian-archive-
# removed-keys.gpg so signatures verify. Package set mirrors the backends
# the original PSC payload links against.
RUN mmdebstrap \
        --architectures=armhf \
        --variant=extract \
        --aptopt='Acquire::Check-Valid-Until "false"' \
        --aptopt='Acquire::AllowInsecureRepositories "true"' \
        --aptopt='Acquire::AllowDowngradeToInsecureRepositories "true"' \
        --aptopt='APT::Get::AllowUnauthenticated "true"' \
        --keyring=/usr/share/keyrings/debian-archive-removed-keys.gpg \
        --include="libasound2-dev,libc6-dev,libgcc-6-dev,libgles2-mesa-dev,libogg-dev,libstdc++-6-dev,libudev-dev,libvorbis-dev,libwayland-dev,libxkbcommon-dev,linux-libc-dev,wayland-protocols,zlib1g-dev" \
        stretch /opt/psc-sysroot \
        "deb http://archive.debian.org/debian stretch main"

# Rewrite absolute symlinks → relative. Otherwise `libc.so` etc. point at
# /lib/arm-linux-gnueabihf/... on the host (which doesn't exist) and the
# linker silently falls back to libc.a / libdl.a, producing broken builds.
# Done by hand because the `symlinks` tool refuses to rewrite links whose
# targets don't already resolve.
RUN cd /opt/psc-sysroot && \
    find . -type l | while read -r link; do \
        target=$(readlink "$link"); \
        case "$target" in \
            /*) \
                dir=$(dirname "$link"); \
                newtarget=$(realpath -m --relative-to="$dir" ".$target"); \
                ln -sfn "$newtarget" "$link"; \
                ;; \
        esac; \
    done

# gcc-6 cross-compiler from Debian Stretch (amd64 host → armhf target).
# gcc ≥ 7 era libstdc++ wants glibc 2.34+ — PSC is on 2.24, so gcc-6 is the
# newest compatible compiler. Stretch's amd64 debs link against glibc 2.24
# but run fine on Ubuntu 24.04 (glibc 2.39 is backward-compatible).
RUN mmdebstrap \
        --architectures=amd64 \
        --variant=extract \
        --aptopt='Acquire::Check-Valid-Until "false"' \
        --aptopt='Acquire::AllowInsecureRepositories "true"' \
        --aptopt='Acquire::AllowDowngradeToInsecureRepositories "true"' \
        --aptopt='APT::Get::AllowUnauthenticated "true"' \
        --keyring=/usr/share/keyrings/debian-archive-removed-keys.gpg \
        --include="g++-6-arm-linux-gnueabihf,gcc-6-arm-linux-gnueabihf,libgcc-6-dev-armhf-cross,libstdc++-6-dev-armhf-cross" \
        stretch /opt/gcc-6 \
        "deb http://archive.debian.org/debian stretch main"

RUN docker-validate gcc

# Strip Stretch's libstdc++ / libgcc_s from the gcc-6 lib bundle. Once
# LD_LIBRARY_PATH points here (needed for libbfd/libisl/etc.), every binary
# loads through this dir first — Stretch's libstdc++.so.6.0.22 doesn't
# satisfy Ubuntu 24.04 binaries that need GLIBCXX_3.4.32. Keep the gcc-6-
# specific deps (bfd, isl, mpc, mpfr, gmp); host libstdc++/libgcc handle
# ABI-compat for both old and new binaries.
RUN cd /opt/gcc-6/usr/lib/x86_64-linux-gnu && \
    rm -f libstdc++.so* libgcc_s.so*

# Symlink-farm gcc-6's cross tree into the sysroot at the cross-style
# location (`/usr/arm-linux-gnueabihf/...`) it expects. Lets gcc-6 +
# --sysroot find Stretch's glibc/libstdc++ and the multiarch headers/libs
# extracted earlier (libwayland, libudev, libGLESv2, ...). Linker scripts
# inside the tree reference `/usr/arm-linux-gnueabihf/lib/...` absolutely;
# under --sysroot they resolve into this farm.
RUN mkdir -p /opt/psc-sysroot/usr/arm-linux-gnueabihf && \
    cp -rs /opt/gcc-6/usr/arm-linux-gnueabihf/. \
           /opt/psc-sysroot/usr/arm-linux-gnueabihf/

# Mirror Stretch's multiarch libs (under /opt/psc-sysroot/{usr/lib,lib}/
# arm-linux-gnueabihf/) into the cross-style /usr/arm-linux-gnueabihf/lib/
# tree so they sit alongside glibc/libstdc++. `-f` clobbers conflicting
# entries (e.g. linker scripts that exist in both); multiarch wins.
RUN cd /opt/psc-sysroot/lib/arm-linux-gnueabihf && \
    for f in *; do \
        ln -sfn "../../../lib/arm-linux-gnueabihf/$f" \
                "/opt/psc-sysroot/usr/arm-linux-gnueabihf/lib/$f"; \
    done && \
    cd /opt/psc-sysroot/usr/lib/arm-linux-gnueabihf && \
    for f in *; do \
        if [ -L "/opt/psc-sysroot/usr/arm-linux-gnueabihf/lib/$f" ] && \
           [ -e "/opt/psc-sysroot/usr/arm-linux-gnueabihf/lib/$f" ]; then \
            continue; \
        fi; \
        ln -sfn "../../lib/arm-linux-gnueabihf/$f" \
                "/opt/psc-sysroot/usr/arm-linux-gnueabihf/lib/$f"; \
    done

# Linker-script path resolution doesn't consistently honor --sysroot for
# absolute paths: Stretch's libc.so contains literal `GROUP ( /usr/arm-linux-
# gnueabihf/lib/...)` references that get looked up at the host FS root.
# Pointing /usr/arm-linux-gnueabihf/{lib,include} into the sysroot makes
# those references resolve. /bin/ under the prefix already exists (from
# binutils-arm-linux-gnueabihf), so we add lib/include symlinks only.
RUN mkdir -p /usr/arm-linux-gnueabihf && \
    ln -sfn /opt/psc-sysroot/usr/arm-linux-gnueabihf/lib \
            /usr/arm-linux-gnueabihf/lib && \
    ln -sfn /opt/psc-sysroot/usr/arm-linux-gnueabihf/include \
            /usr/arm-linux-gnueabihf/include

# Build wayland-scanner from wayland 1.12.0 (matches the sysroot's
# libwayland). Two version mismatches to avoid:
#   * armhf scanner in the sysroot → can't run on amd64 host.
#   * Ubuntu 24.04's host scanner (1.22) → emits wl_proxy_marshal_flags
#     against Stretch's 1.12 headers that lack them.
# We only need the scanner binary (a tiny xml→C codegen).
ARG WAYLAND_VERSION=1.12.0
RUN cd /tmp && \
    wget -q https://wayland.freedesktop.org/releases/wayland-${WAYLAND_VERSION}.tar.xz && \
    tar -xf wayland-${WAYLAND_VERSION}.tar.xz && \
    cd wayland-${WAYLAND_VERSION}/src && \
    printf '#define PACKAGE_VERSION "1.12.0"\n#define HAVE_STRNDUP 1\n' > config.h && \
    gcc -O2 -I. -o wayland-scanner scanner.c wayland-util.c \
        $(pkg-config --cflags --libs expat) && \
    install -D -m755 wayland-scanner /opt/wayland-host/bin/wayland-scanner && \
    ln -sf /opt/wayland-host/bin/wayland-scanner \
           /opt/psc-sysroot/usr/bin/wayland-scanner && \
    /opt/wayland-host/bin/wayland-scanner --version && \
    cd /tmp && rm -rf wayland-${WAYLAND_VERSION}*

# Fail fast if the sysroot is missing critical libraries, or if glibc isn't
# 2.24 (e.g. archive.debian.org served a different version).
RUN docker-validate sysroot

# Cross-compilation toolchain file. CMAKE_SYSROOT plus LIBRARY/INCLUDE/
# PACKAGE = ONLY pin every header and library lookup to the sysroot; PROGRAM
# stays on the host so cmake can still find the cross-compiler. PKG_CONFIG_
# SYSROOT_DIR rewrites .pc prefix paths. We do NOT static-link libgcc/
# libstdc++ — Stretch's libgcc_s.so.1 and libstdc++.so.6.0.22 match the PSC
# ABI, dynamic linking yields the cleanest match.
RUN printf '%s\n' \
    'set(CMAKE_SYSTEM_NAME Linux)' \
    'set(CMAKE_SYSTEM_PROCESSOR arm)' \
    'set(CMAKE_SYSROOT /opt/psc-sysroot)' \
    'set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)' \
    'set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)' \
    'set(CMAKE_FIND_ROOT_PATH /opt/psc-sysroot /usr/local/cross-tools/arm-linux-gnueabihf)' \
    'set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)' \
    'set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)' \
    'set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)' \
    'set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)' \
    'set(ENV{PKG_CONFIG_LIBDIR} "/opt/psc-sysroot/usr/lib/arm-linux-gnueabihf/pkgconfig:/opt/psc-sysroot/usr/share/pkgconfig:/usr/local/cross-tools/arm-linux-gnueabihf/lib/pkgconfig")' \
    'set(ENV{PKG_CONFIG_SYSROOT_DIR} "/opt/psc-sysroot")' \
    > /opt/arm-psc-toolchain.cmake

RUN mkdir -p /usr/local/cross-tools/arm-linux-gnueabihf/lib \
             /usr/local/cross-tools/arm-linux-gnueabihf/include

# Compiler wrappers — both name sets point at Stretch gcc-6 with --sysroot:
#   arm-linux-gnueabihf-{gcc,g++}    — picked up by SDL2 et al. via CC/CXX.
#   armv8-sony-linux-gnueabihf-*     — hard-coded by PSCtoolchainV8.cmake;
#                                       synthesized here so `make arm` works
#                                       unchanged on both Docker and host.
# LD_LIBRARY_PATH gives gcc-6's cc1/cc1plus its Stretch deps (libisl15,
# libmpc3, libmpfr4, libgmp10) — Ubuntu 24.04 ships incompatible versions.
RUN for name in gcc g++; do \
        printf '#!/bin/sh\nexport LD_LIBRARY_PATH=/opt/gcc-6/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}\nexec /opt/gcc-6/usr/bin/arm-linux-gnueabihf-%s-6 --sysroot=/opt/psc-sysroot "$@"\n' \
            "$name" > /usr/local/bin/arm-linux-gnueabihf-$name; \
        cp /usr/local/bin/arm-linux-gnueabihf-$name \
           /usr/local/bin/armv8-sony-linux-gnueabihf-$name; \
        chmod +x /usr/local/bin/arm-linux-gnueabihf-$name \
                 /usr/local/bin/armv8-sony-linux-gnueabihf-$name; \
    done && \
    ln -sf /usr/bin/arm-linux-gnueabihf-ar     /usr/local/bin/armv8-sony-linux-gnueabihf-ar && \
    ln -sf /usr/bin/arm-linux-gnueabihf-gcc-ar /usr/local/bin/armv8-sony-linux-gnueabihf-gcc-ar

# --sysroot on the compiler driver so autoconf feature tests pick it up too
# (CMake projects get it via the toolchain file).
ARG SDL_C_FLAGS="--sysroot=/opt/psc-sysroot -march=armv8-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2"
ARG SDL_LD_FLAGS="--sysroot=/opt/psc-sysroot"

ENV PKG_CONFIG_LIBDIR=/opt/psc-sysroot/usr/lib/arm-linux-gnueabihf/pkgconfig:/opt/psc-sysroot/usr/share/pkgconfig
ENV PKG_CONFIG_SYSROOT_DIR=/opt/psc-sysroot

# Make Stretch's runtime libs (libbfd-2.28, libisl-15, libmpc-3, libmpfr-4,
# libgmp-10) visible to all binaries from /opt/gcc-6, not just the gcc/g++
# wrappers. Without this, autoconf's `ld --version` test silently fails and
# libtool concludes the linker is not GNU ld, disabling shared-library
# production entirely (libSDL2.so gets silently dropped from the build).
ENV LD_LIBRARY_PATH=/opt/gcc-6/usr/lib/x86_64-linux-gnu

WORKDIR /tmp

# SDL2 — shared library shipped via the USB payload at /tmp/lib; consumed by
# autobleem-gui, pscbios, ABFlashKit and other payload apps.
#
# Autotools (not CMake): the known-working PSC archive was configured this
# way, and newer CMake builds reproduced black-screen failures even with
# wl_shell present. Backends match libSDL2-2.0.so.0.12.0:
#   Video:  Wayland (shared) + dummy; X11/KMSDRM/RPi/Vivante/Vulkan off
#   Render: OpenGL ES 1.x + 2.x via EGL (dlopen)
#   Audio:  ALSA (shared) + disk + dummy; PulseAudio/Pipewire/Jack/Sndio off
#   Input:  libudev for hot-plug; DBus/IBus off
RUN wget -q https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz && \
    tar -xf SDL2-${SDL2_VERSION}.tar.gz && \
    cd SDL2-${SDL2_VERSION} && \
    CC=arm-linux-gnueabihf-gcc \
    CXX=arm-linux-gnueabihf-g++ \
    CFLAGS="${SDL_C_FLAGS}" \
    LDFLAGS="${SDL_LD_FLAGS}" \
    ./configure \
        --host=arm-linux-gnueabihf \
        --prefix=/usr/local/cross-tools/arm-linux-gnueabihf \
        --enable-shared \
        --enable-static \
        --disable-video-x11 \
        --enable-video-wayland \
        --enable-wayland-shared \
        --disable-video-wayland-qt-touch \
        --disable-video-kmsdrm \
        --disable-video-rpi \
        --disable-video-vivante \
        --disable-video-vulkan \
        --enable-video-opengl \
        --enable-video-opengles \
        --enable-video-opengles1 \
        --enable-video-opengles2 \
        --enable-alsa \
        --enable-alsa-shared \
        --disable-pulseaudio \
        --disable-jack \
        --disable-sndio \
        --enable-libudev \
        --enable-diskaudio \
        --enable-dummyaudio \
        --enable-video-dummy \
        --disable-dbus \
        --disable-ibus \
        --disable-rpath && \
    make -j$(nproc) && \
    make install && \
    test -f /usr/local/cross-tools/arm-linux-gnueabihf/lib/libSDL2.so && \
    cd /tmp && \
    rm -rf SDL2-${SDL2_VERSION}*

# SDL2_image — shared, vendored PNG + JPG only (AVIF/JXL/TIFF/WebP off).
# Consumed by pscbios and ABFlashKit (libSDL2_image-2.0.so.0).
RUN wget -q https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.tar.gz && \
    tar -xf SDL2_image-${SDL2_IMAGE_VERSION}.tar.gz && \
    cmake -S SDL2_image-${SDL2_IMAGE_VERSION} -B SDL2_image-${SDL2_IMAGE_VERSION}/build \
        -DCMAKE_TOOLCHAIN_FILE=/opt/arm-psc-toolchain.cmake \
        -DCMAKE_C_FLAGS="-march=armv8-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2" \
        -DCMAKE_INSTALL_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf \
        -DSDL2_DIR=/usr/local/cross-tools/arm-linux-gnueabihf/lib/cmake/SDL2 \
        -DSDL2IMAGE_SHARED=ON \
        -DSDL2IMAGE_STATIC=OFF \
        -DSDL2IMAGE_TESTS=OFF \
        -DSDL2IMAGE_VENDORED=ON \
        -DSDL2IMAGE_AVIF=OFF \
        -DSDL2IMAGE_JXL=OFF \
        -DSDL2IMAGE_TIFF=OFF \
        -DSDL2IMAGE_WEBP=OFF \
        -DSDL2IMAGE_PNG=ON \
        -DSDL2IMAGE_JPG=ON && \
    cmake --build SDL2_image-${SDL2_IMAGE_VERSION}/build -j$(nproc) && \
    cmake --install SDL2_image-${SDL2_IMAGE_VERSION}/build && \
    rm -rf SDL2_image-${SDL2_IMAGE_VERSION}*

# SDL2_mixer — backends match libSDL2_mixer-2.0.so.0.0.1: WAV always; OGG via
# libvorbisfile (NEEDED, not dlopen'd); MIDI native parser only; everything
# else off (MOD/FLAC/OPUS/MP3/WavPack/FluidSynth/Timidity).
RUN wget -q https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIXER_VERSION}/SDL2_mixer-${SDL2_MIXER_VERSION}.tar.gz && \
    tar -xf SDL2_mixer-${SDL2_MIXER_VERSION}.tar.gz && \
    cmake -S SDL2_mixer-${SDL2_MIXER_VERSION} -B SDL2_mixer-${SDL2_MIXER_VERSION}/build \
        -DCMAKE_TOOLCHAIN_FILE=/opt/arm-psc-toolchain.cmake \
        -DCMAKE_C_FLAGS="-march=armv8-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2" \
        -DCMAKE_INSTALL_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf \
        -DSDL2_DIR=/usr/local/cross-tools/arm-linux-gnueabihf/lib/cmake/SDL2 \
        -DSDL2MIXER_SHARED=ON \
        -DSDL2MIXER_STATIC=OFF \
        -DSDL2MIXER_TESTS=OFF \
        -DSDL2MIXER_MOD=OFF \
        -DSDL2MIXER_MIDI=ON \
        -DSDL2MIXER_MIDI_NATIVE=ON \
        -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF \
        -DSDL2MIXER_OPUS=OFF \
        -DSDL2MIXER_FLAC=OFF \
        -DSDL2MIXER_WAVPACK=OFF \
        -DSDL2MIXER_OGG=ON \
        -DSDL2MIXER_VORBIS=VORBISFILE \
        -DSDL2MIXER_VORBIS_VORBISFILE_SHARED=OFF \
        -DSDL2MIXER_MP3=OFF && \
    cmake --build SDL2_mixer-${SDL2_MIXER_VERSION}/build -j$(nproc) && \
    cmake --install SDL2_mixer-${SDL2_MIXER_VERSION}/build && \
    rm -rf SDL2_mixer-${SDL2_MIXER_VERSION}*

# SDL2_ttf — vendored freetype, no harfbuzz. Consumed by pscbios + ABFlashKit.
RUN wget -q https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.tar.gz && \
    tar -xf SDL2_ttf-${SDL2_TTF_VERSION}.tar.gz && \
    cmake -S SDL2_ttf-${SDL2_TTF_VERSION} -B SDL2_ttf-${SDL2_TTF_VERSION}/build \
        -DCMAKE_TOOLCHAIN_FILE=/opt/arm-psc-toolchain.cmake \
        -DCMAKE_C_FLAGS="-march=armv8-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2" \
        -DCMAKE_INSTALL_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf \
        -DSDL2_DIR=/usr/local/cross-tools/arm-linux-gnueabihf/lib/cmake/SDL2 \
        -DSDL2TTF_SHARED=ON \
        -DSDL2TTF_STATIC=OFF \
        -DSDL2TTF_TESTS=OFF \
        -DSDL2TTF_VENDORED=ON \
        -DSDL2TTF_HARFBUZZ=OFF && \
    cmake --build SDL2_ttf-${SDL2_TTF_VERSION}/build -j$(nproc) && \
    cmake --install SDL2_ttf-${SDL2_TTF_VERSION}/build && \
    rm -rf SDL2_ttf-${SDL2_TTF_VERSION}*

WORKDIR /build
COPY . /build/

# libchdr — CHD (Compressed Hunks of Data, MAME's compressed disc format)
# support for PS1 games. Static library with bundled zlib/lzma/zstd so the
# PSC needs no extra runtime libs. Tuned for cortex-a35.
#
# Installs to cross-tools:
#   - libchdr-static.a — main CHD reading library
#   - libz.a, liblzma.a, libzstd.a — compression deps
#   - include/libchdr/ — headers
RUN mkdir -p /build/autobleem/libs/libchdr/build && \
    cd /build/autobleem/libs/libchdr/build && \
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=/opt/arm-psc-toolchain.cmake \
        -DCMAKE_C_FLAGS="-march=armv8-a -mtune=cortex-a35 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections -funroll-loops -ftree-vectorize -fno-math-errno -fno-trapping-math -fno-signed-zeros -fprefetch-loop-arrays" \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -Wl,--as-needed -s" \
        -DCMAKE_INSTALL_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf \
        -DBUILD_SHARED_LIBS=OFF \
        -DWITH_SYSTEM_ZLIB=OFF && \
    make -j$(nproc) chdr-static && \
    cp libchdr-static.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp deps/zlib-*/libz.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp deps/lzma-*/liblzma.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp deps/zstd-*/build/cmake/lib/libzstd.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp -r ../include/libchdr /usr/local/cross-tools/arm-linux-gnueabihf/include/

# `make arm` resolves the cross-compiler via PSCtoolchainV8.cmake, which
# hard-codes the `armv8-sony-linux-gnueabihf-*` prefix. The wrappers above
# satisfy that prefix and inject --sysroot, so no env override is needed.
ENV CROSS_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf

ENV GIT_BRANCH=${GIT_BRANCH}
ENV GIT_CHANGED=${GIT_CHANGED}
ENV GIT_COMMIT_HASH=${GIT_COMMIT_HASH}
ENV GIT_VERSION=${GIT_VERSION}

RUN make arm JOBS=$(nproc)

# Catch regressions at image-build time: fail if autobleem-gui references any
# GLIBC symbol above 2.24.
RUN docker-validate glibc-symbols build_arm/autobleem-gui

# Runtime libraries are unpacked into /tmp/lib and selected with
# LD_LIBRARY_PATH; build-container RPATH/RUNPATH entries must not leak into
# the shipped binary.
RUN docker-validate no-rpath build_arm/autobleem-gui

# Repackage libs.tar.gz with the SDL2 family (core/image/mixer/ttf) replaced
# by the builds above. Other libs (libiconv, libmamecd, libogg, libvorbis*)
# are preserved from the original archive so pscbios/ABFlashKit keep working.
RUN mkdir -p /tmp/newlibs && \
    cp -P /usr/local/cross-tools/arm-linux-gnueabihf/lib/libSDL2*.so* /tmp/newlibs/ && \
    tar -xzf /build/autobleem/payload/Autobleem/lib/libs.tar.gz \
        -C /tmp/newlibs \
        --exclude='._*' --exclude='libSDL2*' && \
    cd /tmp/newlibs && \
    tar -czf /build/build_arm/libs.tar.gz . && \
    rm -rf /tmp/newlibs

# ARM-native readelf (statically linked) for deploying onto the PSC. Only the
# readelf sub-program is built to keep compile time short.
RUN cd /tmp && \
    wget -q https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz && \
    tar -xf binutils-${BINUTILS_VERSION}.tar.xz && \
    mkdir binutils-build && \
    cd binutils-build && \
    ../binutils-${BINUTILS_VERSION}/configure \
        --host=arm-linux-gnueabihf \
        --target=arm-linux-gnueabihf \
        --disable-shared \
        --enable-static \
        --disable-nls \
        --disable-werror \
        CC=arm-linux-gnueabihf-gcc \
        CFLAGS="-march=armv8-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2" \
        LDFLAGS="-static" && \
    make all-binutils -j$(nproc) MAKEINFO=true && \
    arm-linux-gnueabihf-strip binutils/readelf && \
    cp binutils/readelf /build/build_arm/readelf && \
    cd /tmp && rm -rf binutils-${BINUTILS_VERSION} binutils-${BINUTILS_VERSION}.tar.xz binutils-build

# Binary is already stripped by the -s linker flag; UPX is for size only.
RUN if [ "${ENABLE_UPX}" = "true" ]; then \
        upx -9 build_arm/autobleem-gui; \
    else \
        echo "Skipping UPX compression"; \
    fi

# Build outputs are in /build/build_arm/.
# Extract with: docker cp <container>:/build/build_arm ./

CMD ["/bin/bash"]
