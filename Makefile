# AutoBleem Build System
#
# This Makefile provides targets for building AutoBleem:
# - Docker builds (recommended for ARM - no toolchain installation required)
# - Native ARM cross-compilation (requires local toolchain)
# - Native x86_64 builds for development/testing
#
# Usage:
#   make              - Build for ARM and local system
#   make build        - Build Docker image for ARM
#   make extract      - Extract ARM binaries from Docker image
#   make sys          - Build for local system (x86_64)
#   make arm          - Build for ARM using local toolchain
#   make clean        - Remove all build artifacts
#   make help         - Show this help

.PHONY: all sys arm mac docker-build docker-extract docker-shell docker-clean english format format-check lint test clean clean-build help

# Docker image name
DOCKER_IMAGE := autobleem-builder

# Number of parallel jobs for make
JOBS := 4

# Default target: build both ARM and local system
all: arm sys

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
	docker build -t $(DOCKER_IMAGE) .
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
	@echo "Creating temporary container..."
	docker create --name autobleem-temp $(DOCKER_IMAGE)
	@echo "Copying build_arm/ directory..."
	docker cp autobleem-temp:/build/build_arm ./
	@echo "Cleaning up temporary container..."
	docker rm autobleem-temp
	@echo "Extraction complete: build_arm/"
	@echo ""
	@echo "Binaries:"
	@ls -lh build_arm/autobleem-gui build_arm/starter 2>/dev/null || echo "  (binaries not found)"

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

# Native build for local system (x86_64) - for development/testing
sys:
	@echo "Building for local system..."
	rm -rf build_sys
	mkdir -p build_sys
	cd build_sys && cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build_sys && make -j $(JOBS)
	@echo "Build complete: build_sys/"

# Native ARM build using local toolchain (requires PSCtoolchainV8)
arm:
	@echo "Building for ARM using local toolchain..."
	rm -rf build_arm
	mkdir -p build_arm
	cd build_arm && cmake -DCMAKE_SYSTEM_PROCESSOR="Arm" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/PSCtoolchainV8.cmake ..
	cd build_arm && make -j $(JOBS)
	@echo "Build complete: build_arm/"

# Native ARM build for macOS using MacToolchain
mac:
	@echo "Building for ARM using Mac toolchain..."
	rm -rf build_arm
	mkdir -p build_arm
	cd build_arm && cmake -DCMAKE_SYSTEM_PROCESSOR="Arm" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/MacToolchain.cmake ..
	cd build_arm && make -j $(JOBS)
	@echo "Build complete: build_arm/"

# Generate English.txt language file from source code strings
english:
	@echo "Generating English.txt..."
	@if [ "$$(uname)" != "Darwin" ]; then \
		grep -r --include=*.cpp --include=*.h --exclude-dir=libs -h -o '_("[^"]*")' . | \
		sed 's/_("//g' | sed 's/")//g' | sed '/^|@lang|$$/d' | sed '/^$$/d' | \
		sort -u | sed G > src/resources/lang/English.txt; \
		echo "Generated src/resources/lang/English.txt"; \
	else \
		echo "Skipping on macOS (grep behavior differs)"; \
	fi

# Format source code with clang-format
format:
	@echo "Formatting source code..."
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "ERROR: clang-format not found!"; \
		echo "Install with: sudo apt-get install clang-format"; \
		exit 1; \
	fi
	@find src/code -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@echo "Formatting complete!"

# Check formatting without modifying files
format-check:
	@echo "Checking code formatting..."
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "ERROR: clang-format not found!"; \
		exit 1; \
	fi
	@find src/code -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
	@echo "Formatting check passed!"

# Run clang-tidy static analysis on all source files
lint:
	@echo "Running clang-tidy static analysis..."
	@if ! command -v clang-tidy >/dev/null 2>&1; then \
		echo "ERROR: clang-tidy not found!"; \
		echo "Install with: sudo apt-get install clang-tidy"; \
		exit 1; \
	fi
	@echo "Checking for build directory..."
	@if [ ! -d "build_sys" ]; then \
		echo "Build directory not found, creating..."; \
		mkdir -p build_sys; \
		cd build_sys && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..; \
	fi
	@echo "Running clang-tidy checks..."
	@find src/code -name "*.cpp" -o -name "*.h" | while read file; do \
		echo "Analyzing $$file..."; \
		clang-tidy "$$file" -p build_sys -- -std=c++11 || true; \
	done
	@echo "Linting complete! Check output above for warnings."

# Run unit tests
test:
	@echo "Running unit tests..."
	@if [ ! -d "build_sys" ]; then \
		echo "Build directory not found. Run 'make sys' first."; \
		exit 1; \
	fi
	@if [ ! -f "build_sys/tests/chd_reader_test" ]; then \
		echo "Tests not built. Building tests..."; \
		cd build_sys && make chd_reader_test; \
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
	@echo "Docker Build Targets (Recommended for ARM):"
	@echo "  make build        Build Docker image for ARM cross-compilation"
	@echo "  make extract      Extract ARM binaries from Docker image"
	@echo "  make shell        Open interactive shell in Docker container"
	@echo "  make clean-build  Remove build artifacts (keeps Docker image)"
	@echo ""
	@echo "Native Build Targets:"
	@echo "  make              Build for ARM and local system"
	@echo "  make sys          Build for local system (x86_64)"
	@echo "  make arm          Build for ARM (requires PSCtoolchainV8)"
	@echo "  make mac          Build for ARM on macOS (requires MacToolchain)"
	@echo ""
	@echo "Code Quality:"
	@echo "  make format       Format source code with clang-format"
	@echo "  make format-check Check formatting without modifying files"
	@echo "  make lint         Run clang-tidy static analysis"
	@echo ""
	@echo "Utility Targets:"
	@echo "  make english      Generate English.txt from source strings"
	@echo "  make test         Run unit tests (requires 'make sys' first)"
	@echo "  make clean        Remove all build artifacts and Docker image"
	@echo "  make help         Show this help"
