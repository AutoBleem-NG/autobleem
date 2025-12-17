cmake_minimum_required(VERSION 3.6)
project(autobleem-gui)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_VERBOSE_MAKEFILE ON)

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
	message(STATUS "Building for ARM")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
		message(STATUS "CXX Compiler Version > 8")
		SET(CMAKE_C_FLAGS "-mfloat-abi=hard -march=armv8-a+simd -Os -s")
		SET(CMAKE_CXX_FLAGS " -mfloat-abi=hard -march=armv8-a+simd  -Os -s")
        message(STATUS "Newer GCC installed")
	else()
		message(STATUS "CXX Compiler Version <= 8")
		SET(CMAKE_C_FLAGS "-mfloat-abi=hard -march=armv7ve -s -Os")
		SET(CMAKE_CXX_FLAGS "-mfloat-abi=hard -march=armv7ve -s -Os")
	endif()

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
        autobleem/code/DebugTimer.cpp
        autobleem/code/DebugTimer.h
        autobleem/code/DirEntry.cpp
        autobleem/code/DirEntry.h
        autobleem/code/engine/cardedit.cpp
        autobleem/code/engine/cardedit.h
        autobleem/code/engine/cfgprocessor.cpp
        autobleem/code/engine/cfgprocessor.h
        autobleem/code/engine/config.cpp
        autobleem/code/engine/config.h
        autobleem/code/engine/coverdb.cpp
        autobleem/code/engine/coverdb.h
        autobleem/code/engine/database.cpp
        autobleem/code/engine/database.h
        autobleem/code/engine/ecmhelper.cpp
        autobleem/code/engine/ecmhelper.h
        autobleem/code/engine/game.cpp
        autobleem/code/engine/game.h
        autobleem/code/engine/GetGameDirHierarchy.cpp
        autobleem/code/engine/GetGameDirHierarchy.h
        autobleem/code/engine/inifile.cpp
        autobleem/code/engine/inifile.h
        autobleem/code/engine/isodir.cpp
        autobleem/code/engine/isodir.h
        autobleem/code/engine/memcard.cpp
        autobleem/code/engine/memcard.h
        autobleem/code/engine/metadata.cpp
        autobleem/code/engine/metadata.h
        autobleem/code/engine/padmapper.cpp
        autobleem/code/engine/padmapper.h
        autobleem/code/engine/scanner.cpp
        autobleem/code/engine/scanner.h
        autobleem/code/engine/serialscanner.cpp
        autobleem/code/engine/serialscanner.h
        autobleem/code/environment.cpp
        autobleem/code/environment.h
        autobleem/code/gui/gui.cpp
        autobleem/code/gui/gui.h
        autobleem/code/gui/gui_about.cpp
        autobleem/code/gui/gui_about.h
        autobleem/code/gui/gui_confirm.cpp
        autobleem/code/gui/gui_confirm.h
        autobleem/code/gui/gui_font.cpp
        autobleem/code/gui/gui_font.h
        autobleem/code/gui/gui_font_wrapper.h
        autobleem/code/gui/gui_keyboard.cpp
        autobleem/code/gui/gui_keyboard.h
        autobleem/code/gui/gui_padTest.cpp
        autobleem/code/gui/gui_padTest.h
        autobleem/code/gui/gui_screen.cpp
        autobleem/code/gui/gui_screen.h
        autobleem/code/gui/gui_scrollWin.cpp
        autobleem/code/gui/gui_scrollWin.h
        autobleem/code/gui/gui_sdl_wrapper.cpp
        autobleem/code/gui/gui_sdl_wrapper.h
        autobleem/code/gui/gui_selectmemcard.cpp
        autobleem/code/gui/gui_selectmemcard.h
        autobleem/code/gui/gui_splash.cpp
        autobleem/code/gui/gui_splash.h
		autobleem/code/gui/SDL_FontCache.h
		autobleem/code/gui/SDL_FontCache.c
        autobleem/code/gui/menus/gui_gameDirMenu.h
        autobleem/code/gui/menus/gui_gameEditorMenu.cpp
        autobleem/code/gui/menus/gui_gameEditorMenu.h
        autobleem/code/gui/menus/gui_gameManagerMenu.cpp
        autobleem/code/gui/menus/gui_gameManagerMenu.h
        autobleem/code/gui/menus/gui_memCardsMenu.cpp
        autobleem/code/gui/menus/gui_memCardsMenu.h
        autobleem/code/gui/menus/gui_menuBase.h
        autobleem/code/gui/menus/gui_optionsMenu.cpp
        autobleem/code/gui/menus/gui_optionsMenu.h
        autobleem/code/gui/menus/gui_optionsMenuBase.cpp
        autobleem/code/gui/menus/gui_optionsMenuBase.h
        autobleem/code/gui/menus/gui_playlistsMenu.h
        autobleem/code/gui/menus/gui_stringMenu.h
        autobleem/code/gui/menus/gui_twoColumnStringMenu.h
        autobleem/code/gui/starfx.cpp
        autobleem/code/gui/starfx.h
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
        autobleem/code/launcher/gui_mc_manager.cpp
        autobleem/code/launcher/gui_mc_manager.h
        autobleem/code/launcher/gui_NotificationLine.cpp
        autobleem/code/launcher/gui_NotificationLine.h
        autobleem/code/launcher/launch_interceptor.cpp
        autobleem/code/launcher/launch_interceptor.h
        autobleem/code/launcher/pcsx_interceptor.cpp
        autobleem/code/launcher/pcsx_interceptor.h
        autobleem/code/launcher/ps_carousel.cpp
        autobleem/code/launcher/ps_carousel.h
        autobleem/code/launcher/ps_centerlabel.cpp
        autobleem/code/launcher/ps_centerlabel.h
        autobleem/code/launcher/ps_game.cpp
        autobleem/code/launcher/ps_game.h
        autobleem/code/launcher/ps_menu.cpp
        autobleem/code/launcher/ps_menu.h
        autobleem/code/launcher/ps_meta.cpp
        autobleem/code/launcher/ps_meta.h
        autobleem/code/launcher/ps_move_bnt.cpp
        autobleem/code/launcher/ps_move_bnt.h
        autobleem/code/launcher/ps_obj.cpp
        autobleem/code/launcher/ps_obj.h
        autobleem/code/launcher/ps_settings_back.cpp
        autobleem/code/launcher/ps_settings_back.h
        autobleem/code/launcher/ps_stateselector.cpp
        autobleem/code/launcher/ps_stateselector.h
        autobleem/code/launcher/ps_static.cpp
        autobleem/code/launcher/ps_static.h
        autobleem/code/launcher/ps_zoom_btn.cpp
        autobleem/code/launcher/ps_zoom_btn.h
        autobleem/code/launcher/ra_integrator.cpp
        autobleem/code/launcher/ra_integrator.h
        autobleem/code/launcher/retboot_interceptor.cpp
        autobleem/code/launcher/retboot_interceptor.h
        autobleem/code/main.cpp
        autobleem/code/main.h
        autobleem/code/unecm.c
        autobleem/code/util.cpp
        autobleem/code/util.h
        autobleem/code/utils/file_utils.cpp
        autobleem/code/utils/random_utils.cpp
        autobleem/code/utils/string_utils.cpp
        autobleem/code/io/binary_reader.cpp
        autobleem/code/system/process_utils.cpp
        autobleem/code/system/storage_info.cpp
        autobleem/code/ver_migration.cpp
        autobleem/code/ver_migration.h
        autobleem/code/util_time.h
		autobleem/code/util_time.cpp
		autobleem/code/gui/abl.c
		autobleem/code/gui/abl.h
		autobleem/code/LightgunGames.h
		autobleem/code/LightgunGames.cpp
		autobleem/code/gui/menus/gui_gameEditorMenu_RA.h
		autobleem/code/gui/menus/gui_gameEditorMenu_RA.cpp)

target_include_directories(autobleem-gui PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs/sqlite>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs/nlohmann>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs/libchdr/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
		)

add_executable(starter
		autobleem/code/starter.cpp
		autobleem/code/util.h
		autobleem/code/util.cpp
		autobleem/code/DirEntry.h
		autobleem/code/DirEntry.cpp
        autobleem/code/engine/inifile.cpp
        autobleem/code/engine/memcard.cpp autobleem/code/engine/memcard.h
		autobleem/code/environment.h
		autobleem/code/environment.cpp
		autobleem/code/utils/file_utils.cpp
		autobleem/code/utils/string_utils.cpp
		)

target_include_directories(starter PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/autobleem/libs>
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


#add_custom_command(TARGET autobleem-gui PRE_BUILD
#	   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#	   VERBATIM
#       COMMAND /bin/bash make_english.txt.sh
#	   )

add_custom_command(TARGET autobleem-gui PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/autobleem/resources $<TARGET_FILE_DIR:autobleem-gui>)


configure_file(autobleem/resources/default.lic default.lic COPYONLY)
configure_file(autobleem/resources/default.png default.png COPYONLY)
configure_file(autobleem/resources/pcsx.cfg pcsx.cfg COPYONLY)

