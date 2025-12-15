# Test suite for AutoBleem
cmake_minimum_required(VERSION 3.6)

# Fetch Google Test from GitHub
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0  # Latest stable release
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories from main project
include_directories(
    ${CMAKE_SOURCE_DIR}/src/code
    ${CMAKE_SOURCE_DIR}/libs/sqlite
    ${CMAKE_SOURCE_DIR}/libs/nlohmann
    ${CMAKE_SOURCE_DIR}/libs/libchdr/include
)

# CHDReader tests
add_executable(
    chd_reader_test
    chd_reader_test.cpp
    ${CMAKE_SOURCE_DIR}/src/code/DirEntry.cpp
    ${CMAKE_SOURCE_DIR}/src/code/util.cpp
    ${CMAKE_SOURCE_DIR}/src/code/environment.cpp
)

target_link_libraries(
    chd_reader_test
    GTest::gtest_main  # Includes main() function
    Threads::Threads
    chdr-static
    ${CMAKE_DL_LIBS}
)

# Register test with CTest
include(GoogleTest)
gtest_discover_tests(chd_reader_test)
