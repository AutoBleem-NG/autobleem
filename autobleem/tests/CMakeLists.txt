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
    ${CMAKE_SOURCE_DIR}/autobleem/code/dir_entry.cpp
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
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/serial_scanner.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/ini_file.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/engine/iso_dir.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/dir_entry.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/io/binary_reader.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/system/process_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/autobleem/code/dir_entry.cpp
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
    ${CMAKE_SOURCE_DIR}/autobleem/code/dir_entry.cpp
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

# Process utilities tests
add_executable(
    process_utils_test
    process_utils_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/system/process_utils.cpp
)

target_link_libraries(
    process_utils_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(process_utils_test)

# Storage info tests
add_executable(
    storage_info_test
    storage_info_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/system/storage_info.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/system/process_utils.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
)

target_link_libraries(
    storage_info_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(storage_info_test)

# Binary reader tests
add_executable(
    binary_reader_test
    binary_reader_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/io/binary_reader.cpp
)

target_link_libraries(
    binary_reader_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(binary_reader_test)

# Time utilities tests
add_executable(
    time_utils_test
    time_utils_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/time_utils.cpp
)

target_link_libraries(
    time_utils_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(time_utils_test)

# Random utilities tests
add_executable(
    random_utils_test
    random_utils_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/random_utils.cpp
)

target_link_libraries(
    random_utils_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(random_utils_test)

# Environment tests
add_executable(
    environment_test
    environment_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/dir_entry.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/string_utils.cpp
)

target_link_libraries(
    environment_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(environment_test)

# Util random index tests (uses new RandomUtils)
add_executable(
    util_test
    util_test.cpp
    ${CMAKE_SOURCE_DIR}/autobleem/code/utils/random_utils.cpp
)

target_link_libraries(
    util_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(util_test)
