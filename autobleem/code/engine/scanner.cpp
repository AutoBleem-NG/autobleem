//
// Created by screemer on 2018-12-15.
//

#include "scanner.h"
#include "disc_suffix.h"
#include "ecm_helper.h"
#include "cfg_processor.h"
#include "metadata.h"
#include "serial_scanner.h"
#include "../lang.h"
#include "../log.h"
#include "../utils/string_utils.h"
#include <algorithm>
#include <map>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "get_game_dir_hierarchy.h"
#include "../environment.h"

using namespace std;

//*******************************
// Scanner::unecm
//*******************************
//
// this routine removes Error Correction files from the bin file to save space
// https://www.lifewire.com/ecm-file-2620956
// https://en.wikipedia.org/wiki/Error_correction_mode
//
void Scanner::unecm(const string &path) {
    for (const DirEntry &entry : FileUtils::dir(path)) {
        if (entry.name[0] == '.')
            continue;
        if (FileUtils::matchExtension(entry.name, EXT_ECM)) {
            Ecmhelper ecm;
            Gui::splash(_("Decompressing ecm") + ":");
            if (ecm.unecm(path + sep + entry.name, path + sep + entry.name.substr(0, entry.name.length() - 4))) {
                FileUtils::removeFile(path + sep + entry.name);
            }
        }
    }
}

//*******************************
// Scanner::updateRegionalDB
//*******************************
void Scanner::updateRegionalDB(GamesHierarchy &gamesHierarchy, Database *db) {
    Gui::splash(_("Updating regional.db..."));
    string path = Env::getWorkingPath() + sep + "autobleem.list";
    ofstream outfile;
    outfile.open(path);
    if (complete) {
        db->beginTransaction();
        for (int i = 0; i < gamesToAddToDB.size(); i++) {
            USBGamePtr data = gamesToAddToDB[i];
            PLOG_INFO << "Inserting game ID: " << i + 1 << " - " << data->title;
            data->gameId = i + 1;
            db->insertGame(data->gameId, data->title, data->publisher, data->players, data->year, data->fullPath + sep,
                           data->saveStatePath + sep, data->memcard);
            if (data->discs.size() == 0)
                PLOG_WARNING << "No discs in game: " << data->title;
            for (int j = 0; j < data->discs.size(); j++) {
                db->insertDisc(i + 1, j + 1, data->discs[j].diskName);
            }
            string gamePath = FileUtils::removeSeparatorFromEndOfPath(data->fullPath);
            string ssPath = FileUtils::removeSeparatorFromEndOfPath(data->saveStatePath);
            outfile << i + 1 << "," << StringUtils::escape(gamePath) << "," << StringUtils::escape(ssPath) << '\n';
        }
        db->commit();
    }
    outfile.flush();
    outfile.close();

    // PLOG_DEBUG << "about to write hierarchy to DB" ;
    gamesHierarchy.printRowDisplayGameInfo(false);

    db->beginTransaction();
    for (auto &row : gamesHierarchy.gameSubDirRows) {
        // PLOG_DEBUG << " write row: " << row->displayRowIndex << ", " << row->subDirName << ", " <<
        // row->displayIndentLevel
        // << ", " << row->gamesToDisplay.size() << endl;
        db->insertSubDirRow(row->displayRowIndex, row->subDirName, row->displayIndentLevel, row->gamesToDisplay.size());

        for (auto &game : row->gamesToDisplay) {
            // PLOG_DEBUG << " write game: " << game->gameDirName << ", " << row->displayRowIndex << ", " <<
            // game->gameId << endl;
            db->insertSubDirGames(row->displayRowIndex, game->gameId);
        }
    }
    db->commit();
}

static const char cue1[] = "FILE \"{binName}\" BINARY\n"
                           "  TRACK 01 MODE2/2352\n"
                           "    INDEX 01 00:00:00\n";
static const char cue2[] = "FILE \"{binName}\" BINARY\n"
                           "  TRACK {track} AUDIO\n"
                           "    INDEX 00 00:00:00\n"
                           "    INDEX 01 00:02:00\n";

//*******************************
// local routines
// *******************************

//*******************************
// repairBinCommaNames
//*******************************
void repairBinCommaNames(const string &path) {
    // TODO: Add support for German diactrics for nex here
    for (DirEntry entry : FileUtils::diru_FilesOnly(path)) {
        if (FileUtils::fixCommaInDirOrFileName(path, &entry)) {
            if (FileUtils::matchExtension(entry.name, EXT_CUE)) {
                // process cue inside
                ifstream is(path + sep + entry.name);
                ofstream os(path + sep + entry.name + ".new");
                string line;
                while (getline(is, line)) {
                    trim(line);
                    if (!line.empty()) {
                        if ((line.rfind("FILE", 0) == 0) || (line.rfind("file", 0) == 0)) {
                            StringUtils::replaceAll(line, ",", "-");
                        }
                    }
                    os << line << endl;
                }

                os.flush();
                os.close();
                is.close();

                FileUtils::removeFile(path + sep + entry.name);
                FileUtils::renameFile(path + sep + entry.name + ".new", path + sep + entry.name);
            }
        }
    }
}

//*******************************
// repairMissingCue
//*******************************
void repairMissingCue(const string &path, const string &folderName) {
    vector<string> binFiles;
    bool hasCue = false;
    DirEntries rootDir = FileUtils::diru_FilesOnly(path);
    for (const DirEntry &entry : rootDir) {
        if (FileUtils::matchExtension(entry.name, EXT_CUE)) {
            hasCue = true;
        }

        if (FileUtils::matchExtension(entry.name, EXT_BIN)) {
            binFiles.push_back(entry.name);
        }
        if (FileUtils::matchExtension(entry.name, EXT_IMG)) {
            binFiles.push_back(entry.name);
        }
    }
    if (!hasCue) {
        string newCueName = path + sep + folderName + EXT_CUE;
        ofstream os;
        os.open(newCueName);
        // let's create new one
        bool first = true;
        int track = 1;
        for (const string &bin : binFiles) {
            string cueElement;
            if (first) {
                cueElement = cue1;
            } else {
                cueElement = cue2;
            }

            StringUtils::replaceAll(cueElement, "{binName}", bin);
            if (track < 10) {
                StringUtils::replaceAll(cueElement, "{track}", "0" + to_string(track));
            } else {
                StringUtils::replaceAll(cueElement, "{track}", to_string(track));
            }
            track++;
            first = false;
            os << cueElement;
        }
        os.flush();
        os.close();
    }
}

//*******************************
// Scanner
// *******************************

//*******************************
// Scanner::moveFolderIfNeeded
//*******************************
void Scanner::moveFolderIfNeeded(const std::string &gameDirName, string gameDataPath, string path) {
    bool gameDataExists = FileUtils::exists(gameDataPath);

    if (gameDataExists) {
        PLOG_INFO << "Game: " << gameDirName << " - Moving GameData to 0.5";
        for (const DirEntry &entryGame : FileUtils::diru(gameDataPath)) {
            string newName = path + sep + gameDirName + sep + entryGame.name;
            string oldName = gameDataPath + sep + entryGame.name;
            PLOG_DEBUG << "Moving: " << oldName << " to: " << newName;
            FileUtils::renameFile(oldName, newName);
        }
    }

    FileUtils::rmDir(gameDataPath);
}

//*******************************
// Scanner::repairBrokenCueFiles
//*******************************
void Scanner::repairBrokenCueFiles(const string &path) {
    vector<string> allBinFiles;
    vector<string> allCues;
    vector<bool> validCue;
    vector<int> cueTracks;

    allBinFiles.clear();
    allCues.clear();
    validCue.clear();
    cueTracks.clear();

    for (const DirEntry &entryGame : FileUtils::diru(path)) {
        if (FileUtils::matchExtension(entryGame.name, EXT_CUE)) {
            allCues.push_back(entryGame.name);
        }

        if (FileUtils::matchExtension(entryGame.name, EXT_BIN)) {
            allBinFiles.push_back(entryGame.name);
        }

        if (FileUtils::matchExtension(entryGame.name, EXT_IMG)) {
            allBinFiles.push_back(entryGame.name);
        }
    }

    for (const string &cue : allCues) {
        ifstream cueStream;

        cueStream.open(path + sep + cue);
        string line;
        bool cueOk = true;
        int bins = 0;
        while (getline(cueStream, line)) {
            line = trim(line);
            if (line.empty())
                continue;
            if (line.substr(0, 4) == "FILE") {
                line = line.substr(6, string::npos);
                line = line.substr(0, line.find('"'));
                bins++;
                if (std::find(allBinFiles.begin(), allBinFiles.end(), line) == allBinFiles.end()) {
                    cueOk = false;
                }
            }
        }
        validCue.push_back(cueOk);
        cueTracks.push_back(bins);
        cueStream.close();
    }

    // now we know cues that are corrupted - regenerate them

    int startPos = 0;
    for (int i = 0; i < allCues.size(); i++) {
        bool cueOk = validCue[i];
        string cuePath = path + sep + allCues[i];
        if (!cueOk) {
            remove(cuePath.c_str());

            ofstream os;
            os.open(cuePath);
            // let's create new one
            bool first = true;
            int track = 1;
            for (int binId = 0; binId != cueTracks[i]; binId++) {
                string cueElement;
                if (first) {
                    cueElement = cue1;
                } else {
                    cueElement = cue2;
                }

                string newBinName = "BinDoesNotExists.bin";
                if ((startPos + binId) < allBinFiles.size()) {
                    newBinName = allBinFiles[startPos + binId];
                }

                StringUtils::replaceAll(cueElement, "{binName}", newBinName);
                if (track < 10) {
                    StringUtils::replaceAll(cueElement, "{track}", "0" + to_string(track));
                } else {
                    StringUtils::replaceAll(cueElement, "{track}", to_string(track));
                }
                track++;
                first = false;
                os << cueElement;
            }
            os.flush();
            os.close();
        }
        startPos += cueTracks[i];
    }
}

//*******************************
// Scanner::scanUSBGamesDirectory
//*******************************
void Scanner::scanUSBGamesDirectory(GamesHierarchy &gamesHierarchy) {
    gamesToAddToDB.clear(); // clear games list
    complete = false;

    PLOG_INFO << "Starting USB games directory scan";
    Gui::splash(_("Scanning..."));

    if (!FileUtils::exists(Env::getPathToSaveStatesDir())) {
        FileUtils::createDir(Env::getPathToSaveStatesDir());
    }

    if (!FileUtils::exists(Env::getPathToMemCardsDir())) {
        FileUtils::createDir(Env::getPathToMemCardsDir());
    }

    USBGames allGames = gamesHierarchy.getAllGames();

    string badGameFilePath = Env::getWorkingPath() + sep + "gamesThatFailedVerifyCheck.txt";
    ofstream badGameFile;
    badGameFile.open(badGameFilePath.c_str(), ios::binary);

#if 0
    int i = 0;
    for (auto game : gamesScanned) {
        PLOG_DEBUG << i++ << ": ";
        if (game)
            PLOG_DEBUG << game->pathName << ", " << game->fullPath ;
        else
            PLOG_DEBUG << "NULL" ;
    }
#endif

    for (USBGamePtr game : allGames) {
        int i = 0;
        if (game)
            PLOG_DEBUG << i++ << ": " << game->gameDirName << ", " << game->fullPath;
        else
            PLOG_DEBUG << i++ << ": NULL";
        repairBinCommaNames(game->fullPath);

        string saveStateDir = Env::getPathToSaveStatesDir() + sep + game->gameDirName;
        FileUtils::createDir(saveStateDir);
        FileUtils::createDir(saveStateDir + sep + "memcards");

        game->folder_id = 0; // this will not be in use;
        game->saveStatePath = Env::getPathToSaveStatesDir() + sep + game->gameDirName + sep;

        Gui::splash(_("Game") + ": " + game->gameDirName);

        string gamePathWithOutSeparator = FileUtils::removeSeparatorFromEndOfPath(game->fullPath);

        moveFolderIfNeeded(game->gameDirName, game->fullPath + sep + GAME_DATA, game->fullPath);

        string gameIniPath = game->fullPath + sep + GAME_INI;

        DirEntries fileEntries = FileUtils::diru_FilesOnly(game->fullPath); // get the list of files once
        if (FileUtils::thereIsAGameFile(fileEntries)) {
            ImageType imageType;
            string gameFile;
            tie(imageType, gameFile) = FileUtils::getGameFile(fileEntries);
            game->imageType = imageType;
            game->gameDataFound = true;

            if (FileUtils::imageTypeUsesACueFile(imageType)) {
                repairMissingCue(game->fullPath, game->gameDirName);
                repairBrokenCueFiles(game->fullPath);
                unecm(game->fullPath);
            }

            // for each file in the game dir
            for (const DirEntry &file : fileEntries) {
                if (StringUtils::compareCaseInsensitive(file.name, GAME_INI)) {
                    game->gameIniFound = true;
                }

                if (StringUtils::compareCaseInsensitive(file.name, PCSX_CFG)) {
                    game->pcsxCfgFound = true;
                }

                if (FileUtils::matchExtension(file.name, EXT_LIC)) {
                    game->licFound = true;
                }
            }

            PLOG_DEBUG << "before recoverMissingFiles() automationUsed=" << game->automationUsed;
            game->recoverMissingFiles();
            PLOG_DEBUG << "after recoverMissingFiles() automationUsed=" << game->automationUsed;

            if (game->gameIniFound)
                game->readIni(gameIniPath); // read it in now in case we need to create or update the serial/region

            // If there was no ini file before, fill in the values from RDB
            // metadata and write Game.ini. Cover art is no longer written to
            // disk — RAIntegrator::findBoxArtPath resolves it from
            // libretro-thumbnails at render time.
            if (!game->gameIniFound || game->automationUsed || (game->discs.size() == 0)) {

                if (game->discs.size() == 0)
                    game->recoverMissingFiles();

                if (game->automationUsed) {
                    game->serial = SerialScanner::scanSerial(game->imageType, game->fullPath + sep, game->firstBinPath);
                    game->region = SerialScanner::serialToRegion(game->serial);
                }

                if (!game->serial.empty()) {
                    Metadata md;
                    if (md.lookupBySerial(game->serial)) {
                        if (game->title == "")
                            game->title = md.title;
                        if (game->publisher == "")
                            game->publisher = md.publisher;
                        if (game->players == 0)
                            game->players = md.players;
                        if (game->year == 0)
                            game->year = md.year;

                        if (game->discs.size() > 0) {
                            game->automationUsed = false;
                        }

                        md.clean();
                    } else {
                        if (game->title == "")
                            game->title = game->gameDirName;
                    }
                }
            }
            game->saveIni(gameIniPath);
            game->readIni(gameIniPath); // the updated iniValues are needed for updateObj
                                        // game->print();

            vector<string> failureReasons;
            if (game->verify(&failureReasons)) {
                gamesToAddToDB.push_back(game);

                string memcardPath = game->saveStatePath + sep + "memcards/";
                if (!FileUtils::exists(memcardPath + "card1.mcd")) {
                    FileUtils::copy(Env::getWorkingPath() + sep + "memcard/card1.mcd", memcardPath + "card1.mcd");
                }
                if (!FileUtils::exists(memcardPath + sep + "card2.mcd")) {
                    FileUtils::copy(Env::getWorkingPath() + sep + "memcard/card1.mcd", memcardPath + "card2.mcd");
                }
                if (!FileUtils::exists(game->saveStatePath + sep + PCSX_CFG)) {
                    FileUtils::copy(Env::getWorkingPath() + sep + PCSX_CFG, game->saveStatePath + sep + PCSX_CFG);
                }
                FileUtils::generateM3UForDirectory(game->fullPath, game->discs[0].cueName);
            } else {
                PLOG_WARNING << "Game failed to verify: " << game->fullPath;
                for (const auto &reason : failureReasons)
                    PLOG_WARNING << "  Reason: " << reason;
                Gui::splash(_("Game failed to verify") + ": " + game->fullPath);
                sleep(3);
                badGameFile << "Game failed to verify: " << game->fullPath << endl;
                for (const auto &reason : failureReasons)
                    badGameFile << "Reason: " << reason << endl;

                // the game did not pass the verify step and was not added to the DB.
                // remove the game everywhere in the gamesHierarchy
                gamesHierarchy.removeGameFromEntireHierarchy(game);
            }
        }
    } // end for each game dir

    badGameFile.close();

    mergeMultiDiscGames(gamesToAddToDB);

    USBGame::sortByTitle(gamesToAddToDB);
    gamesHierarchy.makeGamesToDisplayWhileRemovingChildDuplicates();

    gamesHierarchy.printRowDisplayGameInfo(false);

    string path = Env::getWorkingPath() + sep + "gameHierarchy_afterScanAndRemovingDuplicates.txt";
    ofstream outfile;
    outfile.open(path);
    gamesHierarchy.dumpRowGameInfo(outfile, true);
    outfile << endl << endl;
    gamesHierarchy.dumpRowDisplayGameInfo(outfile, true);
    outfile.close();

    noGamesFoundDuringScan = (gamesToAddToDB.size() == 0);
    complete = true;
    PLOG_INFO << "Scan complete: " << gamesToAddToDB.size() << " games found";
}

namespace {

bool isDiscImageExt(const string &ext) {
    return StringUtils::compareCaseInsensitive(ext, "chd") || StringUtils::compareCaseInsensitive(ext, "pbp") ||
           StringUtils::compareCaseInsensitive(ext, "cue") || StringUtils::compareCaseInsensitive(ext, "bin") ||
           StringUtils::compareCaseInsensitive(ext, "img");
}

} // namespace

//*******************************
// Scanner::mergeMultiDiscGames
//*******************************
// Detects games whose directory name ends with a disc marker (see
// parseDiscSuffix) and merges siblings sharing the same base name into one
// directory. After merging the canonical (lowest-numbered) disc's directory
// contains all disc images plus an .m3u playlist; the redundant disc
// directories are removed and the redundant USBGame entries are dropped
// from `games`.
//
// Skips a group when:
//   - only one disc directory exists for a base name,
//   - the target merged directory exists and isn't the canonical disc,
//   - moving a disc image would overwrite an existing file or a rename fails
//     (the partial state is left intact so the user can recover).
void Scanner::mergeMultiDiscGames(USBGames &games) {
    // Use 0x1f (ASCII unit separator) as an unambiguous key delimiter so two
    // games with identical base names in different parent directories don't
    // collide on the group key.
    static const char kGroupKeySep = '\x1f';

    std::map<string, USBGames> groups;
    for (const USBGamePtr &g : games) {
        const DiscSuffix parsed = parseDiscSuffix(g->gameDirName);
        if (!parsed.matched())
            continue;
        const string parent = FileUtils::getDirNameFromPath(FileUtils::removeSeparatorFromEndOfPath(g->fullPath));
        groups[parent + kGroupKeySep + parsed.base].push_back(g);
    }

    std::vector<USBGamePtr> toRemove;
    for (auto &kv : groups) {
        USBGames &group = kv.second;
        if (group.size() < 2)
            continue;

        std::sort(group.begin(), group.end(), [](const USBGamePtr &a, const USBGamePtr &b) {
            return parseDiscSuffix(a->gameDirName).disc < parseDiscSuffix(b->gameDirName).disc;
        });

        USBGamePtr canonical = group.front();
        const string baseName = parseDiscSuffix(canonical->gameDirName).base;
        const string parentPath =
            FileUtils::getDirNameFromPath(FileUtils::removeSeparatorFromEndOfPath(canonical->fullPath));
        const string targetDir = parentPath + sep + baseName;

        const string canonicalNorm = FileUtils::removeSeparatorFromEndOfPath(canonical->fullPath);
        const string targetNorm = FileUtils::removeSeparatorFromEndOfPath(targetDir);
        if (targetNorm != canonicalNorm && FileUtils::exists(targetDir)) {
            PLOG_WARNING << "multi-disc merge: target '" << targetDir << "' already exists, skipping group for '"
                         << baseName << "'";
            continue;
        }

        if (targetNorm != canonicalNorm) {
            if (!FileUtils::renameFile(canonical->fullPath, targetDir)) {
                PLOG_WARNING << "multi-disc merge: failed to rename '" << canonical->fullPath << "' -> '" << targetDir
                             << "', skipping group";
                continue;
            }
            canonical->fullPath = targetDir;
            canonical->gameDirName = baseName;
            canonical->title = baseName;
        }

        bool mergedAny = false;
        for (size_t i = 1; i < group.size(); i++) {
            USBGamePtr other = group[i];
            bool failed = false;
            for (const DirEntry &e : FileUtils::diru_FilesOnly(other->fullPath)) {
                if (!isDiscImageExt(FileUtils::getFileExtension(e.name)))
                    continue;
                const string from = FileUtils::fixPath(other->fullPath) + sep + e.name;
                const string to = FileUtils::fixPath(targetDir) + sep + e.name;
                if (FileUtils::exists(to)) {
                    PLOG_WARNING << "multi-disc merge: '" << to << "' already exists, skipping move";
                    failed = true;
                    break;
                }
                if (!FileUtils::renameFile(from, to)) {
                    PLOG_WARNING << "multi-disc merge: failed to move '" << from << "' -> '" << to << "'";
                    failed = true;
                    break;
                }
            }
            if (failed)
                continue;
            FileUtils::removeDirAndContents(other->fullPath);
            toRemove.push_back(other);
            mergedAny = true;
        }

        if (mergedAny) {
            // Drop any stale .m3u files (e.g. from a previous per-disc scan)
            // so we end with exactly one playlist named after the merged dir.
            for (const DirEntry &e : FileUtils::diru_FilesOnly(canonical->fullPath)) {
                if (FileUtils::matchExtension(e.name, ".m3u"))
                    FileUtils::removeFile(FileUtils::fixPath(canonical->fullPath) + sep + e.name);
            }
            FileUtils::generateM3UForDirectory(canonical->fullPath, baseName);
            PLOG_INFO << "multi-disc merge: '" << baseName << "' -> " << group.size() << " disc(s) in "
                      << canonical->fullPath;
        }
    }

    if (!toRemove.empty()) {
        games.erase(std::remove_if(games.begin(), games.end(),
                                   [&](const USBGamePtr &g) {
                                       return std::find(toRemove.begin(), toRemove.end(), g) != toRemove.end();
                                   }),
                    games.end());
    }
}

//*******************************
// Scanner::areThereGameFilesInsDir
//*******************************
bool Scanner::areThereGameFilesInDir(const string &path) {
    vector<string> extensions;
    extensions.push_back("pbp");
    extensions.push_back("bin");
    extensions.push_back("cue");
    extensions.push_back("img");
    extensions.push_back("chd");
    //    extensions.push_back("iso");

    // Getting all files in USBGames Dir
    DirEntries globalFileList = FileUtils::diru(path);
    DirEntries fileList = FileUtils::getFilesWithExtension(path, globalFileList, extensions);

    return fileList.size() > 0;
}
