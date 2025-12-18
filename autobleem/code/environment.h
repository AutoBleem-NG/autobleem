#pragma once

#include <string>

struct Environment {
    static std::string getPathToUSBRoot();
    static std::string getPathToAutobleemDir();
    static std::string getPathToAppsDir();
    static std::string getPathToRCDir();
    static std::string getPathToGamesDir();
    static std::string getPathToMemCardsDir();
    static std::string getPathToSaveStatesDir();
    static std::string getPathToSystemDir();
    static std::string getPathToRetroarchDir();
    static std::string getPathToRetroarchPlaylistsDir();
    static std::string getPathToRetroarchCoreFile();
    static std::string getPathToRomsDir();
    static std::string getPathToRegionalDBFile(); // includes the "regional.db" filename
    static std::string getPathToInternalDBFile(); // includes the "internal.db" filename

    // Path behavior varies by argument mode:
    // - 1 arg: returns usb:/Autobleem/bin/autobleem
    // - 2 arg: returns autobleem-gui executable dir
    // - PSC: returns autobleem-gui executable dir
    static std::string getWorkingPath();
    static std::string getSonyPath();          // getWorkingPath() + "/sony"
    static std::string getSonyFontPath();      // getSonyPath() + "/font"
    static std::string getPathToThemesDir();   // 1 arg: usb:/themes, 2 arg: ./themes, PSC: /media/themes
    static std::string getPathToCoversDBDir(); // 1 arg: usb:/Autobleem/bin/db, else: ../db

    static bool autobleemKernel; // true if the kernel is the AutoBleem Kernel
    static bool hiddenMenuEnabled;

    // Parse command line arguments and set up environment paths
    // Returns true on success, false if usage is incorrect
    // - 1 arg (+ program name): USB root path
    // - 2 args (+ program name): regional.db path + games directory path
    static bool parseCommandLineArguments(int argc, char *argv[]);

    // Reset environment paths (for testing)
    static void resetPaths();
};

using Env = Environment;
