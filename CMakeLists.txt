cmake_minimum_required(VERSION 3.10)
project(autobleem-gui)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Generate version.h with build information (git hash, branch, timestamp)
execute_process(
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/autobleem/cmake/generate_version.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add generated files directory to include path (for version.h)
include_directories(${CMAKE_BINARY_DIR}/autobleem/code)

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

	include_directories(autobleem/include
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
include_directories(${CMAKE_SOURCE_DIR}/autobleem/libs)

# Bundled sqlite3 library
add_library(sqlite3 STATIC autobleem/libs/sqlite/sqlite3ab.c)

# Bundled libchdr for CHD file support (local builds only)
if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(autobleem/libs/libchdr EXCLUDE_FROM_ALL)
endif()

# Unit tests (local builds only)
# Tests are built for development/CI environments, not cross-compiled for ARM
if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    option(BUILD_TESTS "Build unit tests" ON)
    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(autobleem/tests)
    endif()
endif()

add_executable(
        autobleem-gui
        autobleem/code/debug_timer.cpp
        autobleem/code/debug_timer.h
        autobleem/code/engine/card_edit.cpp
        autobleem/code/engine/card_edit.h
        autobleem/code/engine/cfg_processor.cpp
        autobleem/code/engine/cfg_processor.h
        autobleem/code/engine/config.cpp
        autobleem/code/engine/config.h
        autobleem/code/engine/cover_db.cpp
        autobleem/code/engine/cover_db.h
        autobleem/code/engine/database.cpp
        autobleem/code/engine/database.h
        autobleem/code/engine/disc_suffix.cpp
        autobleem/code/engine/disc_suffix.h
        autobleem/code/engine/ecm_helper.cpp
        autobleem/code/engine/ecm_helper.h
        autobleem/code/engine/game.cpp
        autobleem/code/engine/game.h
        autobleem/code/engine/get_game_dir_hierarchy.cpp
        autobleem/code/engine/get_game_dir_hierarchy.h
        autobleem/code/engine/ini_file.cpp
        autobleem/code/engine/ini_file.h
        autobleem/code/engine/iso_dir.cpp
        autobleem/code/engine/iso_dir.h
        autobleem/code/engine/mem_card.cpp
        autobleem/code/engine/mem_card.h
        autobleem/code/engine/metadata.cpp
        autobleem/code/engine/metadata.h
        autobleem/code/engine/pad_mapper.cpp
        autobleem/code/engine/pad_mapper.h
        autobleem/code/engine/rdb_reader.cpp
        autobleem/code/engine/rdb_reader.h
        autobleem/code/engine/scanner.cpp
        autobleem/code/engine/scanner.h
        autobleem/code/engine/serial_scanner.cpp
        autobleem/code/engine/serial_scanner.h
        autobleem/code/environment.cpp
        autobleem/code/environment.h
        autobleem/code/gui/gui.cpp
        autobleem/code/gui/gui.h
        autobleem/code/gui/gui_action_text.cpp
        autobleem/code/gui/gui_action_text.h
        autobleem/code/gui/gui_about.cpp
        autobleem/code/gui/gui_about.h
        autobleem/code/gui/gui_confirm.cpp
        autobleem/code/gui/gui_confirm.h
        autobleem/code/gui/gui_font.cpp
        autobleem/code/gui/gui_font_paths.cpp
        autobleem/code/gui/gui_font.h
        autobleem/code/gui/gui_font_wrapper.h
        autobleem/code/gui/gui_layout.cpp
        autobleem/code/gui/gui_layout.h
        autobleem/code/gui/gui_text.cpp
        autobleem/code/gui/gui_text_render.cpp
        autobleem/code/gui/gui_text_utils.cpp
        autobleem/code/gui/gui_text.h
        autobleem/code/gui/gui_keyboard.cpp
        autobleem/code/gui/gui_keyboard.h
        autobleem/code/gui/gui_pad_test.cpp
        autobleem/code/gui/gui_pad_test.h
        autobleem/code/gui/gui_screen.cpp
        autobleem/code/gui/gui_screen.h
        autobleem/code/gui/gui_scroll_win.cpp
        autobleem/code/gui/gui_scroll_win.h
        autobleem/code/gui/gui_sdl_wrapper.cpp
        autobleem/code/gui/gui_sdl_wrapper.h
        autobleem/code/gui/gui_select_mem_card.cpp
        autobleem/code/gui/gui_select_mem_card.h
        autobleem/code/gui/gui_splash.cpp
        autobleem/code/gui/gui_splash.h
		autobleem/code/gui/sdl_font_cache.h
		autobleem/code/gui/sdl_font_cache.c
        autobleem/code/gui/menus/gui_game_dir_menu.h
        autobleem/code/gui/menus/gui_game_editor_menu.cpp
        autobleem/code/gui/menus/gui_game_editor_menu.h
        autobleem/code/gui/menus/gui_game_manager_menu.cpp
        autobleem/code/gui/menus/gui_game_manager_menu.h
        autobleem/code/gui/menus/gui_mem_cards_menu.cpp
        autobleem/code/gui/menus/gui_mem_cards_menu.h
        autobleem/code/gui/menus/gui_menu_base.h
        autobleem/code/gui/menus/gui_options_menu.cpp
        autobleem/code/gui/menus/gui_options_menu.h
        autobleem/code/gui/menus/gui_options_menu_base.cpp
        autobleem/code/gui/menus/gui_options_menu_base.h
        autobleem/code/gui/menus/gui_playlists_menu.h
        autobleem/code/gui/menus/gui_string_menu.h
        autobleem/code/gui/menus/gui_two_column_string_menu.h
        autobleem/code/gui/star_fx.cpp
        autobleem/code/gui/star_fx.h
        autobleem/code/lang.cpp
        autobleem/code/lang.h
        autobleem/code/launcher/emu_interceptor.cpp
        autobleem/code/launcher/emu_interceptor.h
        autobleem/code/launcher/gui_app_start.cpp
        autobleem/code/launcher/gui_app_start.h
        autobleem/code/launcher/gui_btn_guide.cpp
        autobleem/code/launcher/gui_btn_guide.h
        autobleem/code/launcher/gui_launcher.cpp
        autobleem/code/launcher/gui_launcher.h
        autobleem/code/launcher/gui_launcher_loop.cpp
        autobleem/code/launcher/gui_mem_card_manager.cpp
        autobleem/code/launcher/gui_mem_card_manager.h
        autobleem/code/launcher/gui_notification_line.cpp
        autobleem/code/launcher/gui_notification_line.h
        autobleem/code/launcher/launch_interceptor.cpp
        autobleem/code/launcher/launch_interceptor.h
        autobleem/code/launcher/pcsx_interceptor.cpp
        autobleem/code/launcher/pcsx_interceptor.h
        autobleem/code/launcher/ps_carousel.cpp
        autobleem/code/launcher/ps_carousel.h
        autobleem/code/launcher/ps_center_label.cpp
        autobleem/code/launcher/ps_center_label.h
        autobleem/code/launcher/ps_game.cpp
        autobleem/code/launcher/ps_game.h
        autobleem/code/launcher/ps_menu.cpp
        autobleem/code/launcher/ps_menu.h
        autobleem/code/launcher/ps_meta.cpp
        autobleem/code/launcher/ps_meta.h
        autobleem/code/launcher/ps_move_button.cpp
        autobleem/code/launcher/ps_move_button.h
        autobleem/code/launcher/ps_obj.cpp
        autobleem/code/launcher/ps_obj.h
        autobleem/code/launcher/ps_settings_back.cpp
        autobleem/code/launcher/ps_settings_back.h
        autobleem/code/launcher/ps_state_selector.cpp
        autobleem/code/launcher/ps_state_selector.h
        autobleem/code/launcher/ps_static.cpp
        autobleem/code/launcher/ps_static.h
        autobleem/code/launcher/ps_zoom_button.cpp
        autobleem/code/launcher/ps_zoom_button.h
        autobleem/code/launcher/ra_integrator.cpp
        autobleem/code/launcher/ra_integrator.h
        autobleem/code/launcher/thumbnail_lookup.cpp
        autobleem/code/launcher/thumbnail_lookup.h
        autobleem/code/launcher/retboot_interceptor.cpp
        autobleem/code/launcher/retboot_interceptor.h
        autobleem/code/main.cpp
        autobleem/code/main.h
        autobleem/code/unecm.c
        autobleem/code/utils/file_utils.cpp
        autobleem/code/utils/path_utils.cpp
        autobleem/code/utils/random_utils.cpp
        autobleem/code/utils/string_utils.cpp
        autobleem/code/utils/time_utils.cpp
        autobleem/code/io/binary_reader.cpp
        autobleem/code/system/process_utils.cpp
        autobleem/code/system/storage_info.cpp
		autobleem/code/gui/abl.c
		autobleem/code/gui/abl.h
		autobleem/code/lightgun_games.h
		autobleem/code/lightgun_games.cpp
		autobleem/code/gui/menus/gui_game_editor_menu_ra.h
		autobleem/code/gui/menus/gui_game_editor_menu_ra.cpp)

target_include_directories(autobleem-gui PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs/sqlite>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs/nlohmann>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs/libchdr/include>
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
    # Link against bundled libchdr (chdr-static target from autobleem/libs/libchdr)
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
        ${CMAKE_SOURCE_DIR}/autobleem/resources $<TARGET_FILE_DIR:autobleem-gui>)


configure_file(autobleem/resources/default.png default.png COPYONLY)
configure_file(autobleem/resources/pcsx.cfg pcsx.cfg COPYONLY)
