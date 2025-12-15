# AutoBleem ARM Build Environment
# Based on https://github.com/autobleem/autobleem-arm-build
#
# Build: make build (or: docker build -t autobleem-builder .)
# Extract: make extract
FROM ubuntu:18.04

LABEL maintainer="AutoBleem Team"
LABEL description="Docker build environment for AutoBleem - PlayStation Classic payload"

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    bash \
    git \
    cmake \
    gcc-7 \
    g++-7 \
    gcc-7-arm-linux-gnueabihf \
    g++-7-arm-linux-gnueabihf \
    make \
    wget \
    pkg-config \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Create cross-compilation tools directory
RUN mkdir -p /usr/local/cross-tools/arm-linux-gnueabihf/lib \
             /usr/local/cross-tools/arm-linux-gnueabihf/include

# Create symlinks for ARM compilers
RUN ln -sf /usr/bin/arm-linux-gnueabihf-g++-7 /usr/bin/arm-linux-gnueabihf-g++ \
    && ln -sf /usr/bin/arm-linux-gnueabihf-gcc-7 /usr/bin/arm-linux-gnueabihf-gcc

# Create symlinks with the armv8-sony prefix used by PSCtoolchainV8.cmake
RUN ln -sf /usr/bin/arm-linux-gnueabihf-gcc-7 /usr/bin/armv8-sony-linux-gnueabihf-gcc \
    && ln -sf /usr/bin/arm-linux-gnueabihf-g++-7 /usr/bin/armv8-sony-linux-gnueabihf-g++ \
    && ln -sf /usr/bin/arm-linux-gnueabihf-ar /usr/bin/armv8-sony-linux-gnueabihf-ar \
    && ln -sf /usr/bin/arm-linux-gnueabihf-gcc-ar-7 /usr/bin/armv8-sony-linux-gnueabihf-gcc-ar

# Download and install SDL2 libraries for ARM
WORKDIR /tmp
RUN dpkg --add-architecture armhf && \
    mv /etc/apt/sources.list /etc/apt/sources.list.bak && \
    echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu bionic main universe" > /etc/apt/sources.list && \
    echo "deb [arch=amd64] http://archive.ubuntu.com/ubuntu bionic-updates main universe" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports bionic main universe" >> /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports bionic-updates main universe" >> /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y \
        libsdl2-dev:armhf \
        libsdl2-image-dev:armhf \
        libsdl2-mixer-dev:armhf \
        libsdl2-ttf-dev:armhf \
        zlib1g-dev:armhf \
    && rm -rf /var/lib/apt/lists/*

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
RUN mkdir -p /build/libs/libchdr/build && \
    cd /build/libs/libchdr/build && \
    cmake .. \
        -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc-7 \
        -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++-7 \
        -DCMAKE_C_FLAGS="-mfloat-abi=hard -march=armv7ve -Os" \
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

# Build AutoBleem for ARM
RUN make arm JOBS=$(nproc)

# Build outputs are in /build/build_arm/
# Extract with: docker cp <container>:/build/build_arm ./

CMD ["/bin/bash"]
