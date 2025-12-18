#pragma once

#include "utils/string_utils.h"

// Image types for game files - values match existing game.ini format
enum ImageType { IMAGE_NO_GAME_FOUND = -1, IMAGE_BIN = 0, IMAGE_PBP = 1, IMAGE_IMG = 2, IMAGE_CHD = 3 };

// File and directory constants
const char GAME_DATA[] = "GameData";
const char GAME_INI[] = "Game.ini";
const char PCSX_CFG[] = "pcsx.cfg";

// File extensions
const char EXT_PNG[] = ".png";
const char EXT_PBP[] = ".pbp";
const char EXT_ECM[] = ".ecm";
const char EXT_BIN[] = ".bin";
const char EXT_IMG[] = ".img";
const char EXT_CHD[] = ".chd";
const char EXT_CUE[] = ".cue";
const char EXT_LIC[] = ".lic";
