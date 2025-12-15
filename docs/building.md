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

| Target | Description |
|--------|-------------|
| `make` | Build both ARM and local system |
| `make sys` | Build for local system (x86_64) |
| `make arm` | Build for ARM using PSCtoolchainV8 |
| `make mac` | Build for ARM on macOS using MacToolchain |
| `make english` | Generate English.txt from source strings |
| `make clean` | Remove all build directories |
| `make help` | Show all available targets |

## Building for Local Development

```bash
make sys
```

Output binaries are placed in `build_sys/`:
- `autobleem-gui` - Main application
- `starter` - Game launcher wrapper

## Building for PlayStation Classic

### Option 1: Docker Build (Recommended)

The easiest way to build for PlayStation Classic without installing toolchains:

```bash
# Build Docker image and extract binaries
make build extract
```

Output binaries are placed in `build_arm/`:
- `autobleem-gui` - Main UI application
- `starter` - PCSX launcher wrapper
- UI assets (fonts, images, configs) and language files

**Prerequisites:**
- Docker installed ([docker.com](https://docs.docker.com/get-docker/))

**Available commands:**
```bash
make build         # Build Docker image
make extract       # Extract binaries from image
make shell         # Interactive shell in container
make clean-build   # Remove build artifacts
```

### Option 2: Native ARM Cross-Compilation

```bash
make arm
```

Output binaries are placed in `build_arm/`.

Requires the PSC ARM toolchain installed at `/opt/toolchain/armv8-sony-linux-gnueabihf/`. The toolchain file is located at `cmake/PSCtoolchainV8.cmake`.

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

The following libraries are bundled in `libs/`:

- **libchdr** - CHD (Compressed Hunks of Data) disc image format support
- **nlohmann/json** - JSON parsing
- **plog** - Lightweight logging library
- **sqlite3** - Database engine

## Toolchain Files

Located in `cmake/`:

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
