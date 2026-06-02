# Test suite for AutoBleem
cmake_minimum_required(VERSION 3.10)

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
    ${CMAKE_SOURCE_DIR}/code
    ${CMAKE_SOURCE_DIR}/libs/sqlite
    ${CMAKE_SOURCE_DIR}/libs/nlohmann
    ${CMAKE_SOURCE_DIR}/libs/libchdr/include
    ${CMAKE_SOURCE_DIR}/libs
)

# CHDReader tests
add_executable(
    chd_reader_test
    chd_reader_test.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
)

target_compile_definitions(chd_reader_test PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data/"
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
    ${CMAKE_SOURCE_DIR}/code/engine/database.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/serial_scanner.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/ini_file.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/iso_dir.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/io/binary_reader.cpp
    ${CMAKE_SOURCE_DIR}/code/system/process_utils.cpp
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

# RDB reader tests
add_executable(
    rdb_reader_test
    rdb_reader_test.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/rdb_reader.cpp
)

target_link_libraries(
    rdb_reader_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(rdb_reader_test)

# Disc-suffix parser tests
add_executable(
    disc_suffix_test
    disc_suffix_test.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/disc_suffix.cpp
)

target_link_libraries(
    disc_suffix_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(disc_suffix_test)

# Thumbnail-lookup tests (libretro-thumbnails / screenshots / states)
add_executable(
    thumbnail_lookup_test
    thumbnail_lookup_test.cpp
    ${CMAKE_SOURCE_DIR}/code/launcher/thumbnail_lookup.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
)

target_link_libraries(
    thumbnail_lookup_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(thumbnail_lookup_test)

# Log tests
add_executable(
    log_test
    log_test.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
)

target_link_libraries(
    log_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(log_test)

# GUI text/layout helper tests
add_executable(
    gui_text_test
    gui_text_test.cpp
    ${CMAKE_SOURCE_DIR}/code/gui/gui_text_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/gui/gui_font_paths.cpp
    ${CMAKE_SOURCE_DIR}/code/gui/gui_layout.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
)

target_link_libraries(
    gui_text_test
    GTest::gtest_main
    Threads::Threads
    SDL2
)

gtest_discover_tests(gui_text_test)

# Lang (translation) tests
add_executable(
    lang_test
    lang_test.cpp
    ${CMAKE_SOURCE_DIR}/code/lang.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
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
    ${CMAKE_SOURCE_DIR}/code/system/process_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/system/storage_info.cpp
    ${CMAKE_SOURCE_DIR}/code/system/process_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/io/binary_reader.cpp
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
    ${CMAKE_SOURCE_DIR}/code/utils/time_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/utils/random_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
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
    ${CMAKE_SOURCE_DIR}/code/utils/random_utils.cpp
)

target_link_libraries(
    util_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(util_test)

# Main tests (constants and types)
add_executable(
    main_test
    main_test.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
)

target_link_libraries(
    main_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(main_test)

# Debug timer tests
add_executable(
    debug_timer_test
    debug_timer_test.cpp
    ${CMAKE_SOURCE_DIR}/code/debug_timer.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
)

target_link_libraries(
    debug_timer_test
    GTest::gtest_main
    Threads::Threads
    SDL2
)

gtest_discover_tests(debug_timer_test)

# INI file tests
add_executable(
    ini_file_test
    ini_file_test.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/ini_file.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
)

target_link_libraries(
    ini_file_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(ini_file_test)

# Serial scanner tests
add_executable(
    serial_scanner_test
    serial_scanner_test.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/serial_scanner.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/iso_dir.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
    ${CMAKE_SOURCE_DIR}/code/io/binary_reader.cpp
    ${CMAKE_SOURCE_DIR}/code/system/process_utils.cpp
)

target_link_libraries(
    serial_scanner_test
    GTest::gtest_main
    Threads::Threads
    chdr-static
    ${CMAKE_DL_LIBS}
)

gtest_discover_tests(serial_scanner_test)

# ISO directory tests
add_executable(
    iso_dir_test
    iso_dir_test.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/iso_dir.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
)

target_link_libraries(
    iso_dir_test
    GTest::gtest_main
    Threads::Threads
    chdr-static
    ${CMAKE_DL_LIBS}
)

gtest_discover_tests(iso_dir_test)

# Memory card tests
add_executable(
    mem_card_test
    mem_card_test.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/mem_card.cpp
    ${CMAKE_SOURCE_DIR}/code/engine/ini_file.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/code/environment.cpp
)

target_link_libraries(
    mem_card_test
    GTest::gtest_main
    Threads::Threads
)

gtest_discover_tests(mem_card_test)
