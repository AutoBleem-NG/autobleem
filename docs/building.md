# Building AutoBleem-NG

## Prerequisites

### Local Development (x86_64)

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel
```

**macOS:**
```bash
brew install cmake sdl2 sdl2_image sdl2_mixer sdl2_ttf
```

### ARM Cross-Compilation (PlayStation Classic)

Requires the PSC ARM toolchain installed at `/opt/toolchain/armv8-sony-linux-gnueabihf/`.

## Build Targets

### Common Targets

| Target | Description |
|--------|-------------|
| `make` | Build for local system (x86_64) - recommended default |
| `make full` | Build sys + Docker ARM image + extract binaries (recommended for releases) |
| `make test` | Run unit tests (requires `make sys` first) |
| `make clean` | Remove all build directories and Docker image |
| `make help` | Show all available targets |

### Native Build Targets (Incremental)

| Target | Description |
|--------|-------------|
| `make sys` | Build for local system (x86_64) - incremental |
| `make arm` | Build for ARM using PSCtoolchainV8 (requires toolchain) |
| `make mac` | Build for ARM on macOS using MacToolchain |

### Clean Build Targets

| Target | Description |
|--------|-------------|
| `make sys-clean` | Clean rebuild for local system |
| `make arm-clean` | Clean rebuild for ARM |
| `make mac-clean` | Clean rebuild for macOS |

### Docker Build Targets

| Target | Description |
|--------|-------------|
| `make build` | Build Docker image for ARM cross-compilation |
| `make extract` | Extract ARM binaries from Docker image |
| `make shell` | Open interactive shell in Docker container |
| `make clean-build` | Remove build artifacts (keeps Docker image) |

**Note:** By default, build targets use incremental builds (reuse build directory) for faster rebuilds. Use the `-clean` targets when you need a fresh build (e.g., after CMake configuration changes).

## Building for Local Development

```bash
make sys
```

Output binaries are placed in `build_sys/`:
- `autobleem-gui` - Main application
- `starter` - Game launcher wrapper

## Building for PlayStation Classic

### Option 1: Full Build (Recommended for Releases)

Build both local system and ARM binaries in one command:

```bash
make full
```

This runs `make sys` + `make build` + `make extract`, producing:
- `build_sys/` - Local x86_64 binaries (for testing)
- `build_arm/` - ARM binaries (for PlayStation Classic)

### Option 2: Docker Build Only

Build ARM binaries without local build:

```bash
make build extract
```

Output binaries are placed in `build_arm/`:
- `autobleem-gui` - Main UI application
- `starter` - PCSX launcher wrapper
- UI assets (fonts, images, configs) and language files

**Prerequisites:**
- Docker installed ([docker.com](https://docs.docker.com/get-docker/))

### Option 3: Native ARM Cross-Compilation

For users with the PSC ARM toolchain installed:

```bash
make arm
```

Output binaries are placed in `build_arm/`.

Requires the PSC ARM toolchain installed at `/opt/toolchain/armv8-sony-linux-gnueabihf/`. The toolchain file is located at `autobleem/cmake/PSCtoolchainV8.cmake`.

## Build Output Structure

```
build_sys/              # Local x86_64 build
├── autobleem-gui       # Main application binary
├── starter             # Launcher binary
├── default.png         # Default cover image
├── default.lic         # Default license file
└── pcsx.cfg            # PCSX configuration template

build_arm/              # ARM build (same structure)
└── ...
```

## Bundled Libraries

The following libraries are bundled in `autobleem/libs/`:

- **libchdr** - CHD (Compressed Hunks of Data) disc image format support
- **nlohmann/json** - JSON parsing
- **plog** - Lightweight logging library
- **sqlite3** - Database engine

## Toolchain Files

Located in `autobleem/cmake/`:

| File | Description |
|------|-------------|
| `PSCtoolchainV8.cmake` | Primary ARM toolchain for PSC (v8) |
| `MacToolchain.cmake` | ARM cross-compilation on macOS |
| `PSCtoolchainV7.cmake` | Legacy v7 toolchain |
| `PS1Ctoolchain.cmake` | Alternative toolchain |
| `ToolchainServer.cmake` | Server-based toolchain |

## Troubleshooting

### Missing SDL2 libraries

If CMake fails to find SDL2:
```
-- Could NOT find SDL2 (missing: SDL2_LIBRARY SDL2_INCLUDE_DIR)
```

Ensure SDL2 development packages are installed (see Prerequisites above).

### ARM toolchain not found

If the ARM build fails with compiler not found:
```
arm-linux-gnueabihf-gcc: command not found
```

Verify the toolchain is installed at the expected path or update `cmake/PSCtoolchainV8.cmake`.
