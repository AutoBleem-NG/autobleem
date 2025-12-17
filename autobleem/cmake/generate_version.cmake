# Generate version.h with build information
#
# This script is called by CMake to generate version.h with:
# - Git commit hash
# - Git branch
# - Build timestamp
# - Dirty flag (uncommitted changes)

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

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S" UTC)

# Get project version from git tags (fallback to 0.1.0-dev)
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
    set(GIT_DIRTY "false")
    set(GIT_DIRTY_FLAG "")
endif()

# Configure the version header
configure_file(
    ${SOURCE_DIR}/autobleem/code/version.h.in
    ${BINARY_DIR}/autobleem/code/version.h
    @ONLY
)

message(STATUS "Generated version.h: ${GIT_VERSION} (${GIT_BRANCH}@${GIT_COMMIT_HASH}${GIT_DIRTY_FLAG})")
