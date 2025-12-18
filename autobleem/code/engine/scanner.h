//
// Created by screemer on 2018-12-15.
//
#pragma once

#include "../main.h"
#include "game.h"
#include "database.h"
#include "../gui/gui.h"
#include <map>
#include "../utils/file_utils.h"
#include <algorithm>
#include "get_game_dir_hierarchy.h"

//******************
// Scanner
//******************
class Scanner {
  public:
    Scanner() = default;
    USBGames gamesToAddToDB;
    bool forceScan = false;
    bool noGamesFoundDuringScan = false;

    void scanUSBGamesDirectory(GamesHierarchy &gamesHierarchy);
    void repairBrokenCueFiles(const std::string &path);

    void unecm(const std::string &path); // this routine removes Error Correction files from the bin file to save space
    void updateRegionalDB(GamesHierarchy &gamesHierarchy, Database *db);

    static bool areThereGameFilesInDir(const std::string &path);

    Scanner(Scanner const &) = delete;
    Scanner &operator=(Scanner const &) = delete;

    static std::shared_ptr<Scanner> getInstance() {
        static std::shared_ptr<Scanner> s{new Scanner};
        return s;
    }

  private:
    bool complete;
    void moveFolderIfNeeded(const std::string &gameDirName, std::string gameDataPath, std::string path);
};
