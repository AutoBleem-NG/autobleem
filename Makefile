# AutoBleem Build System
#
# This Makefile provides targets for building AutoBleem:
# - Docker builds (default - no host toolchain or SDL2 dev libraries required)
# - Native ARM cross-compilation (requires local toolchain)
# - Native x86_64 builds for development/testing (requires host SDL2 dev libs)
#
# Usage:
#   make              - Build ARM payload via Docker (default - no host deps needed)
#   make full         - Alias for `make build extract`
#   make build        - Build Docker image with ARM binaries inside
#   make extract      - Extract ARM binaries from Docker image to build_arm/
#   make sys          - Build for local system (x86_64); needs libsdl2-dev et al.
#   make sys-clean    - Clean rebuild for local system
#   make arm          - Build for ARM using local toolchain (requires PSCtoolchainV8)
#   make arm-clean    - Clean rebuild for ARM
#   make test         - Run unit tests (requires `make sys` first)
#   make clean        - Remove all build artifacts
#   make help         - Show this help

.PHONY: all full sys sys-clean arm arm-clean mac mac-clean build extract shell clean-build docker-build docker-extract docker-shell docker-clean lang-update lang-validate lang-compare format format-check lint test clean help

# Docker image name
DOCKER_IMAGE := autobleem-builder

# Number of parallel jobs for make
JOBS := 4

# Set ENABLE_UPX=false to leave autobleem-gui uncompressed in Docker builds.
ENABLE_UPX ?= true

# CMake build type
BUILD_TYPE := Release

# Default target: build ARM payload via Docker (no host deps required).
# Use `make sys` explicitly if you want the x86_64 dev build (needs libsdl2-dev).
all: build extract

# Alias for the default Docker-based build pipeline
full: build extract

# Docker build targets for ARM (recommended - no toolchain installation required)

# Build Docker image with ARM cross-compilation environment
build: docker-build

docker-build:
	@echo "Building Docker image for ARM cross-compilation..."
	@if ! command -v docker >/dev/null 2>&1; then \
		echo "ERROR: Docker not found!"; \
		echo "Install from: https://docs.docker.com/get-docker/"; \
		exit 1; \
	fi
	@echo "Extracting git version info..."
	$(eval GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown"))
	$(eval GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown"))
	$(eval GIT_VERSION := $(shell git describe --tags --abbrev=0 2>/dev/null || echo "1.1.0-dev"))
	$(eval GIT_CHANGED := $(shell git diff-index --quiet HEAD -- 2>/dev/null && echo "false" || echo "true"))
	@echo "  Version: $(GIT_VERSION) ($(GIT_BRANCH)@$(GIT_HASH))"
	docker build -t $(DOCKER_IMAGE) \
		--build-arg ENABLE_UPX=$(ENABLE_UPX) \
		--build-arg GIT_COMMIT_HASH=$(GIT_HASH) \
		--build-arg GIT_BRANCH=$(GIT_BRANCH) \
		--build-arg GIT_VERSION=$(GIT_VERSION) \
		--build-arg GIT_CHANGED=$(GIT_CHANGED) \
		.
	@echo "Docker image built successfully: $(DOCKER_IMAGE)"

# Extract built ARM binaries from Docker image
extract: docker-extract

docker-extract:
	@echo "Extracting ARM binaries from Docker image..."
	@if ! docker image inspect $(DOCKER_IMAGE) >/dev/null 2>&1; then \
		echo "ERROR: Docker image '$(DOCKER_IMAGE)' not found!"; \
		echo "Run 'make build' first to create the image."; \
		exit 1; \
	fi
	@echo "Cleaning up any stale containers..."
	@docker rm -f autobleem-temp 2>/dev/null || true
	@echo "Creating temporary container..."
	docker create --name autobleem-temp $(DOCKER_IMAGE)
	@echo "Copying build_arm/ directory..."
	docker cp autobleem-temp:/build/build_arm ./
	@echo "Cleaning up temporary container..."
	docker rm autobleem-temp
	@echo "Extraction complete: build_arm/"
	@echo ""
	@echo "Binaries:"
	@ls -lh build_arm/autobleem-gui build_arm/readelf 2>/dev/null || echo "  (binaries not found)"

# Open interactive shell in Docker container
shell: docker-shell

docker-shell:
	@echo "Opening interactive shell in Docker container..."
	@if ! docker image inspect $(DOCKER_IMAGE) >/dev/null 2>&1; then \
		echo "ERROR: Docker image '$(DOCKER_IMAGE)' not found!"; \
		echo "Run 'make build' first to create the image."; \
		exit 1; \
	fi
	docker run --rm -it $(DOCKER_IMAGE) /bin/bash

# Clean Docker build artifacts (keeps image)
clean-build: docker-clean

docker-clean:
	@echo "Removing Docker build artifacts..."
	rm -rf build_arm
	@echo "Clean complete (Docker image preserved)"
	@echo "To remove Docker image, run: docker rmi $(DOCKER_IMAGE)"

# Native build for local system (x86_64) - incremental
sys:
	@echo "Building for local system (incremental)..."
	@mkdir -p build_sys
	@cd build_sys && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) .. > /dev/null
	cd build_sys && make -j $(JOBS)
	@echo "Build complete: build_sys/"

# Clean rebuild for local system
sys-clean:
	@echo "Clean rebuilding for local system..."
	rm -rf build_sys
	mkdir -p build_sys
	cd build_sys && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ..
	cd build_sys && make -j $(JOBS)
	@echo "Build complete: build_sys/"

# Native ARM build using local toolchain - incremental (requires PSCtoolchainV8)
arm:
	@echo "Building for ARM using local toolchain (incremental)..."
	@mkdir -p build_arm
	@cd build_arm && cmake -DCMAKE_SYSTEM_PROCESSOR="Arm" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=../autobleem/cmake/PSCtoolchainV8.cmake .. > /dev/null
	cd build_arm && make -j $(JOBS)
	@echo "Build complete: build_arm/"

# Clean ARM build using local toolchain
arm-clean:
	@echo "Clean rebuilding for ARM using local toolchain..."
	rm -rf build_arm
	mkdir -p build_arm
	cd build_arm && cmake -DCMAKE_SYSTEM_PROCESSOR="Arm" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=../autobleem/cmake/PSCtoolchainV8.cmake ..
	cd build_arm && make -j $(JOBS)
	@echo "Build complete: build_arm/"

# Native ARM build for macOS - incremental
mac:
	@echo "Building for ARM using Mac toolchain (incremental)..."
	@mkdir -p build_arm
	@cd build_arm && cmake -DCMAKE_SYSTEM_PROCESSOR="Arm" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=../autobleem/cmake/MacToolchain.cmake .. > /dev/null
	cd build_arm && make -j $(JOBS)
	@echo "Build complete: build_arm/"

# Clean ARM build for macOS
mac-clean:
	@echo "Clean rebuilding for ARM using Mac toolchain..."
	rm -rf build_arm
	mkdir -p build_arm
	cd build_arm && cmake -DCMAKE_SYSTEM_PROCESSOR="Arm" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=../autobleem/cmake/MacToolchain.cmake ..
	cd build_arm && make -j $(JOBS)
	@echo "Build complete: build_arm/"

# Update all language files: extract strings, add missing keys, remove obsolete
lang-update:
	@python3 autobleem/scripts/lang_tools.py extract
	@python3 autobleem/scripts/lang_tools.py update --remove-obsolete

# Validate all language files
lang-validate:
	@for f in autobleem/resources/lang/*.txt; do \
		python3 autobleem/scripts/lang_tools.py validate "$$f" -v; \
	done

# Compare all language files against English.txt (show missing translations)
lang-compare:
	@for f in autobleem/resources/lang/*.txt; do \
		if [ "$$(basename $$f)" != "English.txt" ]; then \
			echo "=== $$(basename $$f .txt) ==="; \
			python3 autobleem/scripts/lang_tools.py compare "$$f"; \
			echo; \
		fi; \
	done

# Format source code with clang-format
format:
	@echo "Formatting source code..."
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "ERROR: clang-format not found!"; \
		echo "Install with: sudo apt-get install clang-format"; \
		exit 1; \
	fi
	@find autobleem/code -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} +
	@echo "Formatting complete!"

# Check formatting without modifying files
format-check:
	@echo "Checking code formatting..."
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "ERROR: clang-format not found!"; \
		exit 1; \
	fi
	@find autobleem/code -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format --dry-run --Werror {} +
	@echo "Formatting check passed!"

# Run clang-tidy static analysis on source files
lint:
	@echo "Running clang-tidy static analysis..."
	@if ! command -v clang-tidy >/dev/null 2>&1; then \
		echo "ERROR: clang-tidy not found!"; \
		echo "Install with: sudo apt-get install clang-tidy"; \
		exit 1; \
	fi
	@if [ ! -f "build_sys/compile_commands.json" ]; then \
		echo "Generating compile_commands.json..."; \
		mkdir -p build_sys; \
		cd build_sys && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..; \
	fi
	@echo "Running clang-tidy checks..."
	@find autobleem/code -type f -name "*.cpp" | while read file; do \
		echo "Analyzing $$file..."; \
		clang-tidy "$$file" -p build_sys 2>&1 | grep -v -E "^(Suppressed|Use -header-filter)" || true; \
	done
	@echo "Linting complete!"

# Run unit tests
test:
	@echo "Running unit tests..."
	@if [ ! -d "build_sys" ]; then \
		echo "Build directory not found. Run 'make sys' first."; \
		exit 1; \
	fi
	@if [ ! -d "build_sys/autobleem/tests" ]; then \
		echo "Tests not built. Building..."; \
		cd build_sys && make -j $(JOBS); \
	fi
	@echo ""
	@cd build_sys && ctest --output-on-failure
	@echo ""
	@echo "Tests complete!"

# Clean all build artifacts
clean:
	@echo "Cleaning build directories..."
	rm -rf build_arm build_sys
	@if docker image inspect $(DOCKER_IMAGE) >/dev/null 2>&1; then \
		echo "Removing Docker image $(DOCKER_IMAGE)..."; \
		docker rmi $(DOCKER_IMAGE); \
	fi

# Show help
help:
	@echo "AutoBleem Build System"
	@echo ""
	@echo "Default (Docker-based ARM build - no host deps required):"
	@echo "  make              Build Docker image + extract ARM binaries"
	@echo "  make full         Alias for 'make build extract'"
	@echo ""
	@echo "Docker Build Targets:"
	@echo "  make build        Build Docker image for ARM cross-compilation"
	@echo "  make extract      Extract ARM binaries from Docker image"
	@echo "  make shell        Open interactive shell in Docker container"
	@echo "  make clean-build  Remove build artifacts (keeps Docker image)"
	@echo ""
	@echo "Docker Options:"
	@echo "  ENABLE_UPX=false disables Docker UPX compression"
	@echo ""
	@echo "Native Build Targets (require host toolchain / SDL2 dev libs):"
	@echo "  make sys          Build for local system (x86_64) - incremental"
	@echo "  make arm          Build for ARM (requires PSCtoolchainV8 toolchain)"
	@echo "  make mac          Build for ARM on macOS (requires MacToolchain)"
	@echo ""
	@echo "Native Clean Build Targets (Remove build directory):"
	@echo "  make sys-clean    Clean rebuild for local system"
	@echo "  make arm-clean    Clean rebuild for ARM"
	@echo "  make mac-clean    Clean rebuild for macOS"
	@echo ""
	@echo "Code Quality:"
	@echo "  make format       Format source code with clang-format"
	@echo "  make format-check Check formatting without modifying files"
	@echo "  make lint         Run clang-tidy static analysis"
	@echo ""
	@echo "Language/Localization:"
	@echo "  make lang-update    Extract strings and sync all language files"
	@echo "  make lang-validate  Validate all language files"
	@echo "  make lang-compare   Compare translations against English.txt"
	@echo ""
	@echo "Utility Targets:"
	@echo "  make test         Run unit tests (requires 'make sys' first)"
	@echo "  make clean        Remove all build artifacts and Docker image"
	@echo "  make help         Show this help"
