cmake_minimum_required(VERSION 3.10)
project(autobleem-gui)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Generate version.h with build information (git hash, branch, timestamp)
execute_process(
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/generate_version.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add generated files directory to include path (for version.h)
include_directories(${CMAKE_BINARY_DIR}/code)

# Optional: Enable clang-tidy static analysis
# Usage: cmake -DENABLE_CLANG_TIDY=ON ..
option(ENABLE_CLANG_TIDY "Run clang-tidy static analysis" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found!")
    endif()
endif()

message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" )
	message(STATUS "Building for ARM (PlayStation Classic - Cortex-A35)")
	# PlayStation Classic has ARM Cortex-A35 (ARMv8-A in AArch32 mode)
	#
	# Architecture flags:
	# - march=armv8-a: Target ARMv8-A instruction set
	# - mtune=cortex-a35: Tune for PSC's specific CPU pipeline
	# - mfpu=neon-vfpv4: NEON SIMD with VFPv4 (matches original pcsx-ab exactly)
	# - mfloat-abi=hard: Use hardware FPU, pass floats in VFP registers
	#
	# Optimization flags:
	# - O3: Aggressive optimization
	# - fno-pie: Generate position-dependent code (faster, matches original pcsx-ab)
	# - fomit-frame-pointer: Free up r11 register
	# - ffunction-sections/fdata-sections: Enable dead code elimination
	# - funroll-loops: Unroll loops for better pipelining
	# - ftree-vectorize: Auto-vectorize loops using NEON
	#
	# Math optimizations (safe subset - don't trigger SQLite's __FAST_MATH__ check):
	# - fno-math-errno: Skip errno setting for math functions
	# - fno-trapping-math: Assume FP operations don't trap
	# - fno-signed-zeros: Ignore sign of zero (faster comparisons)
	# - fprefetch-loop-arrays: Prefetch loop data for better cache usage
	#
	SET(CMAKE_C_FLAGS "-march=armv8-a -mtune=cortex-a35 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3 -fno-pie -fomit-frame-pointer -ffunction-sections -fdata-sections -funroll-loops -ftree-vectorize -fno-math-errno -fno-trapping-math -fno-signed-zeros -fprefetch-loop-arrays")
	SET(CMAKE_CXX_FLAGS "-march=armv8-a -mtune=cortex-a35 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3 -fno-pie -fomit-frame-pointer -ffunction-sections -fdata-sections -funroll-loops -ftree-vectorize -fno-math-errno -fno-trapping-math -fno-signed-zeros -fprefetch-loop-arrays")
	SET(CMAKE_EXE_LINKER_FLAGS "-no-pie -Wl,--gc-sections -Wl,--as-needed -s")
	SET(CMAKE_SKIP_RPATH TRUE)
	message(STATUS "C_FLAGS: ${CMAKE_C_FLAGS}")

	include_directories(include
		"/opt/toolchain/armv8-sony-linux-gnueabihf/sysroot/usr/include"
		"/usr/local/cross-tools/arm-linux-gnueabihf/include")
	link_directories(/opt/toolchain/armv8-sony-linux-gnueabihf/sysroot/usr/lib
		/usr/local/cross-tools/arm-linux-gnueabihf/lib
		/usr/lib/arm-linux-gnueabihf)

else()

    message(STATUS "Building for PC CPU type=${CMAKE_BUILD_TYPE}")
	if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
#		if (MEM_DEBUG)	# -DMEM_DEBUG=Extra in Clion does not work!  cmake doesn't see it

#			message(STATUS "Enabling all warnings")
#			# all compiler warnings
#			SET(CMAKE_C_FLAGS "-Wall -W")
#			SET(CMAKE_CXX_FLAGS "-Wall -W")

			message(STATUS "Enabling some warnings")
			# some compiler warnings
			SET(CMAKE_C_FLAGS "-Wreturn-type -Wuninitialized")
			SET(CMAKE_CXX_FLAGS "-Wreturn-type -Wuninitialized")

#			message(STATUS "Enabling additional memory debugging")
			# memory sanitizer with all compiler warnings
#			SET(CMAKE_C_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -ggdb -Wall -W")
#			SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -ggdb -Wall -W")

			# memory sanitizer with compiler warning for not returning a value from a function and uninitialized vars
#			SET(CMAKE_C_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -ggdb -Wreturn-type -Wuninitialized")
#			SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -ggdb -Wreturn-type -Wuninitialized")

			# memory sanitizer with no additonal compiler warnings
#			SET(CMAKE_C_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
#			SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer")

#			SET(LDFLAGS "-fsanitize=address -fno-omit-frame-pointer")

#		endif()
	else()
		message(STATUS "Building Release")
	endif()
endif()

find_package(Threads REQUIRED
             SDL2 REQUIRED)

# Global include directories for bundled libraries
include_directories(${CMAKE_SOURCE_DIR}/libs)

# Bundled sqlite3 library
add_library(sqlite3 STATIC libs/sqlite/sqlite3ab.c)

# Bundled libchdr for CHD file support (local builds only)
if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(libs/libchdr EXCLUDE_FROM_ALL)
endif()

# Unit tests (local builds only)
# Tests are built for development/CI environments, not cross-compiled for ARM
if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    option(BUILD_TESTS "Build unit tests" ON)
    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

add_executable(
        autobleem-gui
        code/debug_timer.cpp
        code/debug_timer.h
        code/engine/card_edit.cpp
        code/engine/card_edit.h
        code/engine/cfg_processor.cpp
        code/engine/cfg_processor.h
        code/engine/config.cpp
        code/engine/config.h
        code/engine/cover_db.cpp
        code/engine/cover_db.h
        code/engine/database.cpp
        code/engine/database.h
        code/engine/disc_suffix.cpp
        code/engine/disc_suffix.h
        code/engine/ecm_helper.cpp
        code/engine/ecm_helper.h
        code/engine/game.cpp
        code/engine/game.h
        code/engine/get_game_dir_hierarchy.cpp
        code/engine/get_game_dir_hierarchy.h
        code/engine/ini_file.cpp
        code/engine/ini_file.h
        code/engine/iso_dir.cpp
        code/engine/iso_dir.h
        code/engine/mem_card.cpp
        code/engine/mem_card.h
        code/engine/metadata.cpp
        code/engine/metadata.h
        code/engine/pad_mapper.cpp
        code/engine/pad_mapper.h
        code/engine/rdb_reader.cpp
        code/engine/rdb_reader.h
        code/engine/scanner.cpp
        code/engine/scanner.h
        code/engine/serial_scanner.cpp
        code/engine/serial_scanner.h
        code/engine/thumbnail_cache.cpp
        code/engine/thumbnail_cache.h
        code/environment.cpp
        code/environment.h
        code/gui/gui.cpp
        code/gui/gui.h
        code/gui/gui_action_text.cpp
        code/gui/gui_action_text.h
        code/gui/gui_about.cpp
        code/gui/gui_about.h
        code/gui/gui_confirm.cpp
        code/gui/gui_confirm.h
        code/gui/gui_font.cpp
        code/gui/gui_font_paths.cpp
        code/gui/gui_font.h
        code/gui/gui_font_wrapper.h
        code/gui/gui_layout.cpp
        code/gui/gui_layout.h
        code/gui/gui_text.cpp
        code/gui/gui_text_render.cpp
        code/gui/gui_text_utils.cpp
        code/gui/gui_text.h
        code/gui/gui_keyboard.cpp
        code/gui/gui_keyboard.h
        code/gui/gui_pad_test.cpp
        code/gui/gui_pad_test.h
        code/gui/gui_screen.cpp
        code/gui/gui_screen.h
        code/gui/gui_scroll_win.cpp
        code/gui/gui_scroll_win.h
        code/gui/gui_sdl_wrapper.cpp
        code/gui/gui_sdl_wrapper.h
        code/gui/gui_select_mem_card.cpp
        code/gui/gui_select_mem_card.h
        code/gui/gui_splash.cpp
        code/gui/gui_splash.h
		code/gui/sdl_font_cache.h
		code/gui/sdl_font_cache.c
        code/gui/menus/gui_game_dir_menu.h
        code/gui/menus/gui_game_editor_menu.cpp
        code/gui/menus/gui_game_editor_menu.h
        code/gui/menus/gui_game_manager_menu.cpp
        code/gui/menus/gui_game_manager_menu.h
        code/gui/menus/gui_mem_cards_menu.cpp
        code/gui/menus/gui_mem_cards_menu.h
        code/gui/menus/gui_menu_base.h
        code/gui/menus/gui_options_menu.cpp
        code/gui/menus/gui_options_menu.h
        code/gui/menus/gui_options_menu_base.cpp
        code/gui/menus/gui_options_menu_base.h
        code/gui/menus/gui_playlists_menu.h
        code/gui/menus/gui_string_menu.h
        code/gui/menus/gui_two_column_string_menu.h
        code/gui/star_fx.cpp
        code/gui/star_fx.h
        code/lang.cpp
        code/lang.h
        code/launcher/emu_interceptor.cpp
        code/launcher/emu_interceptor.h
        code/launcher/gui_app_start.cpp
        code/launcher/gui_app_start.h
        code/launcher/gui_btn_guide.cpp
        code/launcher/gui_btn_guide.h
        code/launcher/gui_launcher.cpp
        code/launcher/gui_launcher.h
        code/launcher/gui_launcher_loop.cpp
        code/launcher/gui_mem_card_manager.cpp
        code/launcher/gui_mem_card_manager.h
        code/launcher/gui_notification_line.cpp
        code/launcher/gui_notification_line.h
        code/launcher/launch_interceptor.cpp
        code/launcher/launch_interceptor.h
        code/launcher/pcsx_interceptor.cpp
        code/launcher/pcsx_interceptor.h
        code/launcher/ps_carousel.cpp
        code/launcher/ps_carousel.h
        code/launcher/ps_center_label.cpp
        code/launcher/ps_center_label.h
        code/launcher/ps_game.cpp
        code/launcher/ps_game.h
        code/launcher/ps_menu.cpp
        code/launcher/ps_menu.h
        code/launcher/ps_meta.cpp
        code/launcher/ps_meta.h
        code/launcher/ps_move_button.cpp
        code/launcher/ps_move_button.h
        code/launcher/ps_obj.cpp
        code/launcher/ps_obj.h
        code/launcher/ps_settings_back.cpp
        code/launcher/ps_settings_back.h
        code/launcher/ps_state_selector.cpp
        code/launcher/ps_state_selector.h
        code/launcher/ps_static.cpp
        code/launcher/ps_static.h
        code/launcher/ps_zoom_button.cpp
        code/launcher/ps_zoom_button.h
        code/launcher/ra_integrator.cpp
        code/launcher/ra_integrator.h
        code/launcher/thumbnail_lookup.cpp
        code/launcher/thumbnail_lookup.h
        code/launcher/retboot_interceptor.cpp
        code/launcher/retboot_interceptor.h
        code/main.cpp
        code/main.h
        code/unecm.c
        code/utils/file_utils.cpp
        code/utils/path_utils.cpp
        code/utils/random_utils.cpp
        code/utils/string_utils.cpp
        code/utils/time_utils.cpp
        code/io/binary_reader.cpp
        code/system/process_utils.cpp
        code/system/storage_info.cpp
		code/gui/abl.c
		code/gui/abl.h
		code/lightgun_games.h
		code/lightgun_games.cpp
		code/gui/menus/gui_game_editor_menu_ra.h
		code/gui/menus/gui_game_editor_menu_ra.cpp)

target_include_directories(autobleem-gui PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/sqlite>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/nlohmann>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/libchdr/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
		)

# Link libraries for autobleem-gui
#
# libchdr provides CHD (Compressed Hunks of Data) disc image support.
# Compression libraries (zlib, lzma, zstd) are required by libchdr.
#
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(CROSS_LIB_DIR "/usr/local/cross-tools/arm-linux-gnueabihf/lib")
    target_link_libraries(autobleem-gui
             Threads::Threads
             SDL2
             SDL2_image
             SDL2_mixer
             SDL2_ttf
             sqlite3
             ${CROSS_LIB_DIR}/libchdr-static.a
             ${CROSS_LIB_DIR}/libz.a
             ${CROSS_LIB_DIR}/liblzma.a
             ${CROSS_LIB_DIR}/libzstd.a
             ${CMAKE_DL_LIBS}
    )
else()
    # Link against bundled libchdr (chdr-static target from libs/libchdr)
    # The chdr-static target already links its internal deps (lzma, zlib, zstd)
    target_link_libraries(autobleem-gui
             Threads::Threads
             SDL2
             SDL2_image
             SDL2_mixer
             SDL2_ttf
             sqlite3
             chdr-static
             ${CMAKE_DL_LIBS}
    )
endif()

add_custom_command(TARGET autobleem-gui PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:autobleem-gui>)


configure_file(resources/default.png default.png COPYONLY)
configure_file(resources/pcsx.cfg pcsx.cfg COPYONLY)
