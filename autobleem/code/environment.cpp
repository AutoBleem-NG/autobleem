#include "environment.h"
#include "DirEntry.h"
#include "log.h"
#include <dirent.h>
#include <unistd.h>
#include <climits>
#include <cassert>

using namespace std;

// these strings are initialized by main.cpp.  once initialized they should not be modified.
// they are not declared in the header to prevent outside code from accidentally modifying them.

// passing a single arg on the debug command line instead of passing two args is optional.
//
// in single arg mode you pass the path to the root of the flash USB drive.  in this mode, as much as possible, files
// from the USB drive are used instead of files in the debug build environment.  this allows you to debug the files
// on the USB drive.  this also allows someone to send you the contents of a USB drive to debug a
// problem.  you will use the UI theme, the menu themes, the cover db files, the regional db file, the .prev file,
// the retroarch playlists, the rom files, the lang files, and the config.ini file, etc.

bool private_singleArgPassed = false;
string private_pathToUSBDrive;
string private_pathToGamesDir;
string private_pathToRegionalDBFile;
string private_pathToInternalDBFile;
bool Env::autobleemKernel = false;
bool Env::hiddenMenuEnabled = false;

//*******************************
// Environment:: One Liners
//*******************************

string Environment::getPathToUSBRoot() { return private_pathToUSBDrive; }

string Environment::getPathToAutobleemDir() { return private_pathToUSBDrive + sep + "Autobleem"; }

string Environment::getPathToAppsDir() { return private_pathToUSBDrive + sep + "Apps"; }

std::string Environment::getPathToRCDir() { return getPathToAutobleemDir() + sep + "rc"; }

string Environment::getPathToGamesDir() { return private_pathToGamesDir; }

string Environment::getPathToMemCardsDir() { return private_pathToGamesDir + sep + "!MemCards"; }

string Environment::getPathToSaveStatesDir() { return private_pathToGamesDir + sep + "!SaveStates"; }

string Environment::getPathToSystemDir() { return private_pathToUSBDrive + sep + "System"; }

string Environment::getPathToRetroarchDir() { return private_pathToUSBDrive + sep + "retroarch"; }

string Environment::getPathToRetroarchPlaylistsDir() { return getPathToRetroarchDir() + sep + "playlists"; }

string Environment::getPathToRetroarchCoreFile() {
    return getPathToRetroarchDir() + sep + "cores/km_pcsx_rearmed_neon_libretro.so";
}

string Environment::getPathToRomsDir() { return private_pathToUSBDrive + sep + "roms"; }

// includes the "regional.db" filename
string Environment::getPathToRegionalDBFile() { return private_pathToRegionalDBFile; }

// includes the "internal.db" filename
string Environment::getPathToInternalDBFile() { return private_pathToInternalDBFile; }

//*******************************
// Environment::getWorkingPath
// 1 arg: "usb:/Autobleem/bin/autobleem"
// 2 arg: autobleem-gui executable dir
// PSC: autobleem-gui executable dir
//*******************************
string Environment::getWorkingPath() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    if (private_singleArgPassed) {
        string path = private_pathToUSBDrive + sep + "Autobleem/bin/autobleem";
        return path;
    } else {
        char temp[PATH_MAX];
        return (getcwd(temp, sizeof(temp)) ? string(temp) : string(""));
    }
#else
    char temp[PATH_MAX];
    return (getcwd(temp, sizeof(temp)) ? string(temp) : string(""));
#endif
}

//*******************************
// Environment::getSonyPath
// 1 arg: "usb:/Autobleem/bin/autobleem/sony"
// 2 arg: autobleem-gui executable dir + sep + "sony"
// PSC: autobleem-gui executable dir
//*******************************
string Environment::getSonyPath() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    string sonypath = getWorkingPath() + sep + "sony";
    assert(DirEntry::exists(sonypath));
    return sonypath;
#else
    return "/usr/sony/share/data";
#endif
}

//*******************************
// Environment::getSonyFontPath
// 1 arg: "usb:/Autobleem/bin/autobleem/sony/font"
// 2 arg: autobleem-gui executable dir + sep + "sony" + sep + "font"
// PSC: autobleem-gui executable dir
//*******************************
string Environment::getSonyFontPath() { return getSonyPath() + sep + "font"; }

#if 0
//*******************************
// Environment::getPathToWorkingPathFile
// return path to file in working path
// 1 arg: "usb:/Autobleem/bin/autobleem/filename"
// 2 arg: autobleem-gui executable dir/filename
// PSC: autobleem-gui executable dir/filename
//*******************************
string Environment::getPathToWorkingPathFile(const std::string &filename) {
    return getWorkingPath() + sep + filename;
}
#endif

//*******************************
// Environment::getPathToThemesDir
// 1 arg: "usb:/themes", 2 arg: "./themes"
// PSC: "/media/themes"
//*******************************
string Environment::getPathToThemesDir() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    if (private_singleArgPassed) {
        string path = private_pathToUSBDrive + sep + "themes";
        return path;
    } else {
        string path = getWorkingPath() + sep + "themes";
        return path;
    }
#else
    string path = "/media/themes";
    return path;
#endif
}

//*******************************
// Environment::getPathToCoversDBDir
// 1 arg: "usb:/Autobleem/bin/db", 2 arg: "../db"
// PSC: "../db"
//*******************************
string Environment::getPathToCoversDBDir() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    if (private_singleArgPassed) {
        string path = private_pathToUSBDrive + sep + "Autobleem/bin/db";
        return path;
    } else {
        string path = "../db";
        return path;
    }
#else
    string path = "../db";
    return path;
#endif
}

// Configuration constants for argument parsing
namespace {
const char *const DATABASES_SUBDIR = "System/Databases";
const char *const GAMES_SUBDIR = "Games";
const char *const INTERNAL_DB_FILENAME = "internal.db";
const char *const REGIONAL_DB_FILENAME = "regional.db";
const char *const INTERNAL_DB_MEDIA_PATH = "/media/System/Databases/internal.db";
const char *const MSG_USAGE = "Usage: autobleem-gui /path/to/database.db /path/to/games";

inline string getDatabaseSubpath(const char *filename) { return string(DATABASES_SUBDIR) + sep + filename; }
} // namespace

// Reset environment paths (for testing)
void Environment::resetPaths() {
    private_singleArgPassed = false;
    private_pathToUSBDrive.clear();
    private_pathToGamesDir.clear();
    private_pathToRegionalDBFile.clear();
    private_pathToInternalDBFile.clear();
}

// Parse command line arguments and set up environment paths
bool Environment::parseCommandLineArguments(int argc, char *argv[]) {
    if (argc == 1 + 1) {
        // the single arg is the path to the USB drive
        private_singleArgPassed = true;
        private_pathToUSBDrive = argv[1];
        private_pathToRegionalDBFile = private_pathToUSBDrive + sep + getDatabaseSubpath(REGIONAL_DB_FILENAME);
        private_pathToInternalDBFile = private_pathToUSBDrive + sep + getDatabaseSubpath(INTERNAL_DB_FILENAME);
        private_pathToGamesDir = private_pathToUSBDrive + sep + GAMES_SUBDIR;
        return true;
    }

    if (argc == 1 + 2) {
        // the two args are the path to the regional.db file and the path to the /Games dir on the USB drive
        private_singleArgPassed = false;
        private_pathToRegionalDBFile = argv[1];
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
        // in development, use internal.db in the same dir as the app
        private_pathToInternalDBFile = INTERNAL_DB_FILENAME;
#else
        private_pathToInternalDBFile = INTERNAL_DB_MEDIA_PATH;
#endif
        private_pathToGamesDir = argv[2];
        private_pathToUSBDrive = DirEntry::getDirNameFromPath(private_pathToGamesDir);
        return true;
    }

    PLOG_ERROR << MSG_USAGE;
    return false;
}
