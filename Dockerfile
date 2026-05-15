# AutoBleem ARM Build Environment
# Based on https://github.com/autobleem/autobleem-arm-build
#
# Build: make build (or: docker build -t autobleem-builder .)
# Extract: make extract
#
# Xenial ships glibc 2.23, just below PSC firmware's 2.24. Newer Ubuntu releases
# pull symbols (memfd_create, rebased expf/logf/powf) tagged GLIBC_2.27+ that
# fail to resolve on PSC.
FROM ubuntu:16.04

LABEL maintainer="AutoBleem Team"
LABEL description="Docker build environment for AutoBleem - PlayStation Classic payload"

# Build arguments for version info (passed from host where .git exists)
ARG GIT_COMMIT_HASH=unknown
ARG GIT_BRANCH=unknown
ARG GIT_VERSION=1.1.0-dev
ARG GIT_CHANGED=false

# UPX version for binary compression
ARG UPX_VERSION=5.0.2

# Xenial's CMake 3.5 lacks `cmake -S/-B` (3.13+) and `cmake --install` (3.15+)
# used by the SDL2 family builds below.
ARG CMAKE_VERSION=3.28.6

# SDL2 pinned to 2.0.12: matches libSDL2-2.0.so.0.12.0 from the original PSC
# payload. Newer builds can init cleanly yet still leave the PSC at a black
# screen, so we stay on the known-good revision.
#
# SDL2_mixer 2.6.x: first line where Mix_LoadWAV is a real exported symbol
# (in 2.0.x it was a macro for Mix_LoadWAV_RW); autobleem-gui calls it directly.
# 2.6 is also the last line ABI-compatible with SDL2 2.0.12 — 2.8.x requires
# SDL2 2.0.24+.
ARG SDL2_VERSION=2.0.12
ARG SDL2_IMAGE_VERSION=2.6.3
ARG SDL2_MIXER_VERSION=2.6.3
ARG SDL2_TTF_VERSION=2.20.2

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# gcc-5 is xenial's default. PSC ships libstdc++.so.6.0.22 (GCC 6.2.0), which
# is forward-compatible with binaries linked against 6.0.21 from gcc-5.
RUN apt-get update && apt-get install -y \
    bash \
    git \
    cmake \
    gcc-5 \
    g++-5 \
    gcc-5-arm-linux-gnueabihf \
    g++-5-arm-linux-gnueabihf \
    make \
    wget \
    xz-utils \
    pkg-config \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Download and install UPX for binary compression
RUN wget -q https://github.com/upx/upx/releases/download/v${UPX_VERSION}/upx-${UPX_VERSION}-amd64_linux.tar.xz && \
    tar -xf upx-${UPX_VERSION}-amd64_linux.tar.xz && \
    cp upx-${UPX_VERSION}-amd64_linux/upx /usr/local/bin/ && \
    chmod +x /usr/local/bin/upx && \
    rm -rf upx-${UPX_VERSION}-amd64_linux upx-${UPX_VERSION}-amd64_linux.tar.xz

# Modern CMake into /usr/local — takes precedence over apt's 3.5.
RUN wget -q https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
    sh cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/usr/local && \
    rm cmake-${CMAKE_VERSION}-linux-x86_64.sh

# Create cross-compilation tools directory
RUN mkdir -p /usr/local/cross-tools/arm-linux-gnueabihf/lib \
             /usr/local/cross-tools/arm-linux-gnueabihf/include

# Create symlinks for ARM compilers
RUN ln -sf /usr/bin/arm-linux-gnueabihf-g++-5 /usr/bin/arm-linux-gnueabihf-g++ \
    && ln -sf /usr/bin/arm-linux-gnueabihf-gcc-5 /usr/bin/arm-linux-gnueabihf-gcc

# Create symlinks with the armv8-sony prefix used by PSCtoolchainV8.cmake
RUN ln -sf /usr/bin/arm-linux-gnueabihf-gcc-5 /usr/bin/armv8-sony-linux-gnueabihf-gcc \
    && ln -sf /usr/bin/arm-linux-gnueabihf-g++-5 /usr/bin/armv8-sony-linux-gnueabihf-g++ \
    && ln -sf /usr/bin/arm-linux-gnueabihf-ar /usr/bin/armv8-sony-linux-gnueabihf-ar \
    && ln -sf /usr/bin/arm-linux-gnueabihf-gcc-ar-5 /usr/bin/armv8-sony-linux-gnueabihf-gcc-ar

# ARM dependencies for the SDL2 source builds below. Backend choice (Wayland,
# OpenGL ES, ALSA, libudev) follows what the original PSC SDL2 2.0.12 linked
# against. libogg/libvorbis are present because SDL2_mixer NEEDED-links libvorbisfile.
WORKDIR /tmp
RUN dpkg --add-architecture armhf && \
    mv /etc/apt/sources.list /etc/apt/sources.list.bak && \
    echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu xenial main universe" > /etc/apt/sources.list && \
    echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu xenial-updates main universe" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports xenial main universe" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports xenial-updates main universe" >> /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y \
        libgles2-mesa-dev:armhf \
        libasound2-dev:armhf \
        libudev-dev:armhf \
        libwayland-dev:armhf \
        libxkbcommon-dev:armhf \
        wayland-protocols \
        libogg-dev:armhf \
        libvorbis-dev:armhf \
    && rm -rf /var/lib/apt/lists/*

# SDL2 ARM cross-compilation toolchain file. FIND_ROOT_PATH modes are NEVER so
# cmake uses PKG_CONFIG_PATH and CMAKE_PREFIX_PATH directly instead of
# prepending a sysroot.
RUN printf 'set(CMAKE_SYSTEM_NAME Linux)\n\
set(CMAKE_SYSTEM_PROCESSOR arm)\n\
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc-5)\n\
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++-5)\n\
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n\
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)\n\
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)\n\
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)\n\
set(ENV{PKG_CONFIG_PATH} "/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/local/cross-tools/arm-linux-gnueabihf/lib/pkgconfig:/usr/share/pkgconfig")\n\
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/local/cross-tools/arm-linux-gnueabihf/lib/pkgconfig:/usr/share/pkgconfig")\n' \
    > /tmp/arm-sdl-toolchain.cmake

# Common ARM compile flags for all SDL2 builds
ARG SDL_C_FLAGS="-march=armv8-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2"

# Build SDL2 as a shared library shipped via the USB payload at /tmp/lib —
# autobleem-gui, pscbios, ABFlashKit and any other payload app pick it up.
#
# Use autotools rather than CMake: the known-working PSC archive was configured
# this way, and the black-screen failure reproduced with newer SDL2 builds even
# with wl_shell present. Matching the original generation is lower risk.
#
# Backends match libSDL2-2.0.so.0.12.0:
#   Video:  Wayland (shared) + dummy; X11/KMSDRM/RPi/Vivante/Vulkan off
#   Render: OpenGL ES 1.x + 2.x via EGL (dlopen)
#   Audio:  ALSA (shared) + disk + dummy; PulseAudio/Pipewire/Jack/Sndio off
#   Input:  libudev for hot-plug
#   DBus/IBus off
RUN wget -q https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz && \
    tar -xf SDL2-${SDL2_VERSION}.tar.gz && \
    cd SDL2-${SDL2_VERSION} && \
    PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig \
    PKG_CONFIG_LIBDIR=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig \
    CC=arm-linux-gnueabihf-gcc-5 \
    CXX=arm-linux-gnueabihf-g++-5 \
    CFLAGS="${SDL_C_FLAGS}" \
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
    cd /tmp && \
    rm -rf SDL2-${SDL2_VERSION}*

# Built shared for the USB payload (pscbios and ABFlashKit also link
# libSDL2_image-2.0.so.0).
RUN wget -q https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.tar.gz && \
    tar -xf SDL2_image-${SDL2_IMAGE_VERSION}.tar.gz && \
    cmake -S SDL2_image-${SDL2_IMAGE_VERSION} -B SDL2_image-${SDL2_IMAGE_VERSION}/build \
        -DCMAKE_TOOLCHAIN_FILE=/tmp/arm-sdl-toolchain.cmake \
        -DCMAKE_C_FLAGS="${SDL_C_FLAGS}" \
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

# Built shared for the USB payload. Backends match libSDL2_mixer-2.0.so.0.0.1:
#   WAV  always on
#   OGG  via libvorbisfile (NEEDED, not dlopen'd)
#   MIDI native parser only (no FluidSynth, no external Timidity)
#   MOD/FLAC/OPUS/MP3/WavPack off
RUN wget -q https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIXER_VERSION}/SDL2_mixer-${SDL2_MIXER_VERSION}.tar.gz && \
    tar -xf SDL2_mixer-${SDL2_MIXER_VERSION}.tar.gz && \
    cmake -S SDL2_mixer-${SDL2_MIXER_VERSION} -B SDL2_mixer-${SDL2_MIXER_VERSION}/build \
        -DCMAKE_TOOLCHAIN_FILE=/tmp/arm-sdl-toolchain.cmake \
        -DCMAKE_C_FLAGS="${SDL_C_FLAGS}" \
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

# Built shared for the USB payload (pscbios and ABFlashKit also link
# libSDL2_ttf-2.0.so.0).
RUN wget -q https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.tar.gz && \
    tar -xf SDL2_ttf-${SDL2_TTF_VERSION}.tar.gz && \
    cmake -S SDL2_ttf-${SDL2_TTF_VERSION} -B SDL2_ttf-${SDL2_TTF_VERSION}/build \
        -DCMAKE_TOOLCHAIN_FILE=/tmp/arm-sdl-toolchain.cmake \
        -DCMAKE_C_FLAGS="${SDL_C_FLAGS}" \
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

# Copy source code into container (before libchdr build)
WORKDIR /build
COPY . /build/

# Build libchdr for ARM - provides CHD (Compressed Hunks of Data) support
# CHD is MAME's compressed disc image format, commonly used for PS1 games.
#
# Build as static library to avoid runtime dependencies on the PlayStation Classic.
# Use libchdr's bundled compression libraries (zlib, lzma, zstd) since cross-compiling
# system versions is problematic.
#
# Outputs installed to cross-tools directory:
#   - libchdr-static.a: Main CHD reading library
#   - libz.a, liblzma.a, libzstd.a: Compression libraries (required by libchdr)
#   - include/libchdr/: Header files
RUN mkdir -p /build/autobleem/libs/libchdr/build && \
    cd /build/autobleem/libs/libchdr/build && \
    cmake .. \
        -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc-5 \
        -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++-5 \
        -DCMAKE_C_FLAGS="-march=armv8-a -mtune=cortex-a35 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections -funroll-loops -ftree-vectorize -fno-math-errno -fno-trapping-math -fno-signed-zeros -fprefetch-loop-arrays" \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -Wl,--as-needed -s" \
        -DCMAKE_INSTALL_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf \
        -DBUILD_SHARED_LIBS=OFF \
        -DWITH_SYSTEM_ZLIB=OFF \
    && make -j$(nproc) chdr-static && \
    mkdir -p /usr/local/cross-tools/arm-linux-gnueabihf/lib && \
    cp libchdr-static.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp deps/zlib-*/libz.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp deps/lzma-*/liblzma.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp deps/zstd-*/build/cmake/lib/libzstd.a /usr/local/cross-tools/arm-linux-gnueabihf/lib/ && \
    cp -r ../include/libchdr /usr/local/cross-tools/arm-linux-gnueabihf/include/

# Set environment variables for cross-compilation
ENV PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig
ENV CROSS_PREFIX=/usr/local/cross-tools/arm-linux-gnueabihf

# Set version info as environment variables for CMake
ENV GIT_COMMIT_HASH=${GIT_COMMIT_HASH}
ENV GIT_BRANCH=${GIT_BRANCH}
ENV GIT_VERSION=${GIT_VERSION}
ENV GIT_CHANGED=${GIT_CHANGED}

# Build AutoBleem for ARM
RUN make arm JOBS=$(nproc)

# Repackage libs.tar.gz with the SDL2 family (core/image/mixer/ttf) replaced by
# the builds above. Other libs (libiconv, libmamecd, libogg, libvorbis*) are
# preserved from the original archive so pscbios/ABFlashKit keep working.
RUN mkdir -p /tmp/newlibs && \
    cp -P /usr/local/cross-tools/arm-linux-gnueabihf/lib/libSDL2*.so* /tmp/newlibs/ && \
    tar -xzf /build/autobleem/payload/Autobleem/lib/libs.tar.gz \
        -C /tmp/newlibs \
        --exclude='._*' --exclude='libSDL2*' && \
    cd /tmp/newlibs && \
    tar -czf /build/build_arm/libs.tar.gz . && \
    rm -rf /tmp/newlibs

# Compress binary with UPX for smaller size and faster loading
# Note: Binary is already stripped by -s linker flag
RUN upx -9 build_arm/autobleem-gui

# Build outputs are in /build/build_arm/
# Extract with: docker cp <container>:/build/build_arm ./

CMD ["/bin/bash"]
