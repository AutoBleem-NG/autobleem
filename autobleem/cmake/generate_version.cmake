# Generate version.h with build information
#
# This script is called by CMake to generate version.h with:
# - Git commit hash
# - Git branch
# - Build timestamp
# - Dirty flag (uncommitted changes)
#
# Version info can come from:
# 1. Environment variables (for Docker builds where .git is not available)
# 2. Git commands (for local builds)
# 3. Fallback defaults

# Check for environment variables first (Docker build)
if(DEFINED ENV{GIT_COMMIT_HASH} AND NOT "$ENV{GIT_COMMIT_HASH}" STREQUAL "unknown")
    set(GIT_COMMIT_HASH "$ENV{GIT_COMMIT_HASH}")
    set(GIT_BRANCH "$ENV{GIT_BRANCH}")
    set(GIT_VERSION "$ENV{GIT_VERSION}")
    if("$ENV{GIT_CHANGED}" STREQUAL "true")
        set(GIT_CHANGED "true")
        set(GIT_CHANGED_FLAG " (changed)")
    else()
        set(GIT_CHANGED "false")
        set(GIT_CHANGED_FLAG "")
    endif()
    message(STATUS "Using version info from environment variables")
else()
    # Get git commit hash (short)
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Get git branch
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Check if working directory has uncommitted changes
    execute_process(
        COMMAND git diff-index --quiet HEAD --
        WORKING_DIRECTORY ${SOURCE_DIR}
        RESULT_VARIABLE GIT_CHANGED_RESULT
        ERROR_QUIET
    )

    if(GIT_CHANGED_RESULT EQUAL 0)
        set(GIT_CHANGED "false")
        set(GIT_CHANGED_FLAG "")
    else()
        set(GIT_CHANGED "true")
        set(GIT_CHANGED_FLAG " (changed)")
    endif()

    # Get project version from git tags (fallback to 1.1.0-dev)
    execute_process(
        COMMAND git describe --tags --abbrev=0
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(NOT GIT_VERSION)
        set(GIT_VERSION "1.1.0-dev")
    endif()

    # Fallback values if git is not available
    if(NOT GIT_COMMIT_HASH)
        set(GIT_COMMIT_HASH "unknown")
        set(GIT_BRANCH "unknown")
        set(GIT_CHANGED "false")
        set(GIT_CHANGED_FLAG "")
    endif()
endif()

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S" UTC)

# Configure the version header
configure_file(
    ${SOURCE_DIR}/autobleem/code/version.h.in
    ${BINARY_DIR}/autobleem/code/version.h
    @ONLY
)

message(STATUS "Generated version.h: ${GIT_VERSION} (${GIT_BRANCH}@${GIT_COMMIT_HASH}${GIT_DIRTY_FLAG})")
