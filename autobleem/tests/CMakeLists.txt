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
    ${CMAKE_SOURCE_DIR}/autobleem/code
    ${CMAKE_SOURCE_DIR}/autobleem/libs/sqlite
    ${CMAKE_SOURCE_DIR}/autobleem/libs/nlohmann
    ${CMAKE_SOURCE_DIR}/autobleem/libs/libchdr/include
    ${CMAKE_SOURCE_DIR}/autobleem/libs
)

# CHDReader tests
add_executable(
    chd_reader_test
    chd_reader_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/DirEntry.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/util.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
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

# Database tests
add_executable(
    database_test
    database_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/database.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/serialscanner.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/inifile.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/isodir.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/DirEntry.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/util.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
)

target_link_libraries(
    database_test
    GTest::gtest_main
    Threads::Threads
    sqlite3
    chdr-static
    ${CMAKE_DL_LIBS}
)

gtest_discover_tests(database_test)

# Log tests
add_executable(
    log_test
    log_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/DirEntry.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/util.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
)

target_link_libraries(
    log_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(log_test)

# Lang (translation) tests
add_executable(
    lang_test
    lang_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/lang.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/DirEntry.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/util.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
)

target_link_libraries(
    lang_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(lang_test)

# String utilities tests
add_executable(
    string_utils_test
    string_utils_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
)

target_link_libraries(
    string_utils_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(string_utils_test)

# Path utilities tests
add_executable(
    path_utils_test
    path_utils_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/path_utils.cpp
)

target_link_libraries(
    path_utils_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(path_utils_test)

# File utilities tests
add_executable(
    file_utils_test
    file_utils_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/file_utils.cpp
)

target_link_libraries(
    file_utils_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(file_utils_test)
