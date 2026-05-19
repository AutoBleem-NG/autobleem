// Original author: screemer
#include "main.h"

#include <unistd.h>

#include <iostream>
#include <memory>

#include "engine/cover_db.h"
#include "engine/database.h"
#include "engine/get_game_dir_hierarchy.h"
#include "engine/ini_file.h"
#include "engine/mem_card.h"
#include "engine/scanner.h"
#include "environment.h"
#include "gui/abl.h"
#include "gui/gui.h"
#include "lang.h"
#include "launcher/emu_interceptor.h"
#include "launcher/gui_app_start.h"
#include "launcher/launch_interceptor.h"
#include "launcher/pcsx_interceptor.h"
#include "launcher/ra_integrator.h"
#include "launcher/retboot_interceptor.h"
#include "log.h"
#include "system/process_utils.h"
#include "ver_migration.h"
#include "version.h"

using namespace std;

Database *db = nullptr;

// Search for games with supported extension and move to sub-dir.
// Returns true if any files moved into sub-dirs.
bool copyGameFilesInGamesDirToSubDirs(const string &path) {
    bool ret = false;
    string fileExt;
    string filenameWE;
    vector<string> extensions;
    vector<string> binList;

    extensions.push_back("pbp");
    extensions.push_back("cue");
    extensions.push_back("chd");

    // Getting all files in USBGames Dir
    DirEntries globalFileList = FileUtils::diru(path);
    DirEntries fileList = FileUtils::getFilesWithExtension(path, globalFileList, extensions);

    // On first run, we won't process bin/img files, as cue file may handle a part of them
    for (const auto &entry : fileList) {
        Gui::splash(_("Moving") + ": " + entry.name);
        fileExt = FileUtils::getFileExtension(entry.name);
        filenameWE = FileUtils::getFileNameWithoutExtension(entry.name);
        // Checking if file exists
        if (access((path + sep + entry.name).c_str(), F_OK) != -1) {
            if (fileExt == "cue") {
                binList = FileUtils::cueToBinList(path + sep + entry.name);
                if (!binList.empty()) {
                    // Create directory for game
                    FileUtils::createDir(path + sep + filenameWE);
                    // Move cue file
                    FileUtils::renameFile(path + "/" + entry.name, path + sep + filenameWE + "/" + entry.name);
                    // Move bin files
                    for (const auto &bin : binList) {
                        Gui::splash(_("Moving") + ": " + bin);
                        FileUtils::renameFile(path + sep + bin, path + sep + filenameWE + sep + bin);
                    }
                    ret = true;
                }
            } else {
                FileUtils::createDir(path + sep + filenameWE);

                FileUtils::renameFile(path + sep + entry.name, path + sep + filenameWE + sep + entry.name);
                ret = true;
            }
        }
    }

    // Next we will read only bin and img files
    extensions.clear();
    extensions.push_back("img");
    extensions.push_back("bin");
    fileList = FileUtils::getFilesWithExtension(path, globalFileList, extensions);
    for (const auto &entry : fileList) {
        Gui::splash(_("Moving") + ": " + entry.name);
        fileExt = FileUtils::getFileExtension(entry.name);
        filenameWE = FileUtils::getFileNameWithoutExtension(entry.name);
        // Checking if file exists
        if (access((path + sep + entry.name).c_str(), F_OK) != -1) {
            FileUtils::createDir(path + sep + filenameWE);
            FileUtils::renameFile(path + sep + entry.name, path + sep + filenameWE + sep + entry.name);
            ret = true;
        }
    }
    return ret;
}

int scanGames(GamesHierarchy &gamesHierarchy) {
    shared_ptr<Gui> gui(Gui::getInstance());
    shared_ptr<Scanner> scanner(Scanner::getInstance());

    // create the db tables in case they don't already exist
    if (!db->createInitialDatabase()) {
        PLOG_ERROR << "Error creating db structure";

        return EXIT_FAILURE;
    };

    // save the history and last_played for any games that have it
    PsGames gamesWithHistoryOrLastPlayed;
    if (db->getGames(&gamesWithHistoryOrLastPlayed)) {
        auto noDataToSave = [](PsGamePtr psGame) { return (psGame->history == 0) && (psGame->last_played == 0); };
        auto it = remove_if(begin(gamesWithHistoryOrLastPlayed), end(gamesWithHistoryOrLastPlayed), noDataToSave);
        gamesWithHistoryOrLastPlayed.erase(it, end(gamesWithHistoryOrLastPlayed));
    }

    // delete all data in all tables
    if (!db->truncate()) {
        gui->drawText("ERROR IN DB");
        sleep(1);
        return EXIT_FAILURE;
    }

    scanner->scanUSBGamesDirectory(gamesHierarchy);
    scanner->updateRegionalDB(gamesHierarchy, gui->db);

    // restore the history and last_played for any games that still exist in the db
    PsGames psGames;
    db->getGames(&psGames);
    for (PsGamePtr savedGame : gamesWithHistoryOrLastPlayed) {
        auto it = find_if(begin(psGames), end(psGames),
                          [&](PsGamePtr psGame) { return psGame->folder == savedGame->folder; });
        if (it != end(psGames)) {
            db->updateHistory((*it)->gameId, savedGame->history);
            db->updateDatePlayed((*it)->gameId, savedGame->last_played);
        }
    }

    gui->drawText(_("Total") + ": " + to_string(scanner->gamesToAddToDB.size()) + " " + _("games scanned") + ".");
    sleep(1);
    scanner->gamesToAddToDB.clear();
    return EXIT_SUCCESS;
}

// Rewrite the gamelist.xml file for Emulation Station to find PS1 cover files
void rewriteGamelistXml() {
    // this file was used during 0.9.0 testing.  it must be removed or ES will use it by mistake.
    FileUtils::removeFile(Env::getPathToGamesDir() + sep + "gamelist.xml");

    string path = Env::getPathToRetroarchDir() + sep + "retroboot/emulationstation/.emulationstation/gamelists/psx";
    FileUtils::createDir(path);
    string filePath = path + sep + "gamelist.xml";

    FileUtils::removeFile(filePath);

    PsGames currentGames;
    Gui::getInstance()->db->getGames(&currentGames);

    ofstream xml;
    xml.open(filePath.c_str(), ios::binary);

    xml << "<?xml version=\"1.0\"?>" << endl;
    xml << "<gameList>" << endl;

    auto makeGamesPathRelative = [](const string &oldPath) -> string {
        string newPath = oldPath;
        size_t pos = newPath.find("/Games");
        if (pos != string::npos) {
            newPath.erase(0, pos - 1 + sizeof("/Games"));
            newPath = "." + newPath;
        }
        return newPath;
    };

    for (const auto &game : currentGames) {
        xml << "\t<game>" << endl;

        xml << "\t\t<path>" << makeGamesPathRelative(game->folder) << "</path>" << endl;
        xml << "\t\t<name>" << game->title << "</name>" << endl;
        xml << "\t\t<desc>" << game->title << "</desc>" << endl;
        string imagePath = game->folder + sep + game->base + ".png";
        xml << "\t\t<image>" << makeGamesPathRelative(imagePath) << "</image>" << endl;

        xml << "\t</game>" << endl;
    }
    xml << "</gameList>" << endl;

    xml.close();
}

int main(int argc, char *argv[]) {
    // Set up environment paths FIRST (needed for log file location)
    if (!Env::parseCommandLineArguments(argc, argv)) {
        return EXIT_FAILURE;
    }

    // Initialize logger - Log::init creates parent directories if needed
    string logPath = Env::getPathToUSBRoot() + sep + "System" + sep + "Logs" + sep + "autobleem-ng.log";
    Log::init(logPath);

    // Log version and build information
    PLOG_INFO << Version::FULL_VERSION << " - built " << Version::BUILD_TIMESTAMP << " UTC";

    PLOG_INFO << "AutoBleem starting, argc=" << argc;
    for (int i = 0; i < argc; i++) {
        PLOG_DEBUG << "  argv[" << i << "]=" << argv[i];
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    Env::autobleemKernel = FileUtils::exists("/autobleem");
    shared_ptr<Lang> lang(Lang::getInstance());

    // now that Environment is setup, routines that need the paths can be called
    shared_ptr<Gui> gui(Gui::getInstance());
    shared_ptr<Scanner> scanner(Scanner::getInstance());
    gui->mapper.init();
    lang->load(gui->cfg.inifile.values["language"]);

    PLOG_DEBUG << "Path to USB: " << Env::getPathToUSBRoot();
    PLOG_DEBUG << "Path to Games: " << Env::getPathToGamesDir();
    PLOG_DEBUG << "Path to Regional DB: " << Env::getPathToRegionalDBFile();

    unique_ptr<Coverdb> coverdb(new Coverdb());
    gui->coverdb = coverdb.get();

    db = new Database();
    if (!db->connect(Env::getPathToRegionalDBFile())) {
        delete db;
        db = nullptr;
        return EXIT_FAILURE;
    }
    gui->db = db;
    db->createInitialDatabase();

    // if the /System/Databases/internal.db doesn't exist make a copy from the PSC
    PLOG_INFO << "Importing internal games from PSC to USB";
    System::executeCommand("/media/Autobleem/rc/backup_internal.sh");

    // add favorites and history columns to internal.db if the column doesn't exist
    unique_ptr<Database> internalDB(new Database());
    if (!internalDB->connect(Env::getPathToInternalDBFile())) {
        delete db;
        db = nullptr;
        return EXIT_FAILURE;
    }
    gui->internalDB = internalDB.get();
    gui->internalDB->addFavoriteColumn();
    gui->internalDB->addHistoryColumn();
    gui->internalDB->addLastPlayedColumn();
    gui->internalDB->addPlayUsingRAColumn();

    string pathToGamesDir = Env::getPathToGamesDir();

    Memcard memcardOperation(pathToGamesDir);
    memcardOperation.restoreAll(Env::getPathToSaveStatesDir());

    string prevPath = Env::getWorkingPath() + sep + "autobleem.prev";
    bool prevFileExists = FileUtils::exists(prevPath);

    bool useEmulationStation = false;
    string rbCfgPath = Env::getPathToRetroarchDir() + sep + "retroboot/retroboot.cfg";
    if (FileUtils::exists(rbCfgPath)) {
        Inifile rbCfg;
        rbCfg.load(rbCfgPath);
        useEmulationStation = (rbCfg.values["use_emulationstation"] == "1");
    }
    bool gamelistXmlExists =
        !useEmulationStation ||
        FileUtils::exists(Env::getPathToRetroarchDir() + sep +
                          "retroboot/emulationstation/.emulationstation/gamelists/psx/gamelist.xml");

    GamesHierarchy gamesHierarchy;
    gamesHierarchy.getHierarchy(pathToGamesDir);

    USBGames allGames = gamesHierarchy.getAllGames();
    USBGame::sortByFullPath(allGames);

    bool autobleemPrevOutOfDate = gamesHierarchy.gamesDoNotMatchAutobleemPrev(prevPath);
    bool thereAreRawGameFilesInGamesDir = scanner->areThereGameFilesInDir(pathToGamesDir);

    if (!prevFileExists || !gamelistXmlExists || thereAreRawGameFilesInGamesDir || autobleemPrevOutOfDate) {
        scanner->forceScan = true;
    }

    gui->display(scanner->forceScan, pathToGamesDir, db, false);

    if (thereAreRawGameFilesInGamesDir)
        copyGameFilesInGamesDirToSubDirs(pathToGamesDir); // the gui->display needs to be up first

    while (gui->menuOption == MENU_OPTION_SCAN || gui->menuOption == MENU_OPTION_START) {

        gui->menuSelection();
        gui->saveSelection();
        if (gui->menuOption == MENU_OPTION_SCAN) {
            gamesHierarchy.getHierarchy(pathToGamesDir);
            // write the prev file now.  if you call it after scanGames the games that failed to verify will have been
            // removed from the hierarchy and it force you to rescan on every boot.
            gamesHierarchy.writeAutobleemPrev(prevPath);
            scanGames(gamesHierarchy);
            rewriteGamelistXml();

            gui->forceScan = false;
        }

        if (gui->menuOption == MENU_OPTION_START) {
            PLOG_INFO << "Starting game: " << gui->runningGame->title;
            gui->finish();

            int numtimesopened, frequency, channels;
            Uint16 format;
            numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
            for (int i = 0; i < numtimesopened; i++) {
                Mix_CloseAudio();
            }
            while (Mix_QuerySpec(&frequency, &format, &channels)) {
                Mix_CloseAudio();
            }

            gui->mapper.flushPads();

            gui->saveSelection();
            unique_ptr<EmuInterceptor> interceptor;
            if (gui->runningGame->foreign) {
                if (!gui->runningGame->app) {
                    interceptor.reset(new RetroArchInterceptor());
                } else {
                    interceptor.reset(new LaunchInterceptor());
                }
            } else {
                if (gui->emuMode == EMU_PCSX) {
                    interceptor.reset(new PcsxInterceptor());
                } else {
                    interceptor.reset(new RetroArchInterceptor());
                }
            }

            interceptor->memcardIn(gui->runningGame);
            interceptor->prepareResumePoint(gui->runningGame, gui->resumepoint);
            interceptor->execute(gui->runningGame, gui->resumepoint);
            interceptor->memcardOut(gui->runningGame);

            bool reloadFavHist = gui->runningGame->foreign || (gui->emuMode != EMU_PCSX);
            if (reloadFavHist) {
                auto ra = RAIntegrator::getInstance();
                ra->reloadFavorites(); // they could have changed
                ra->reloadHistory();   // they could have changed
            }

            usleep(300 * 1000);

            gui->mapper.probePads();
            gui->runningGame.reset(); // replace with shared_ptr pointing to nullptr
            gui->startingGame = false;
            // remove all events if something left
            SDL_PumpEvents();
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

            gui->display(false, pathToGamesDir, db, true);
        }
    }
    db->disconnect();
    delete db;
    db = nullptr;

    internalDB->disconnect();

    PLOG_INFO << "AutoBleem shutting down";
    Gui::splash(_("Loading... Please Wait..."));
    gui->finish();
    SDL_Quit();

    return EXIT_SUCCESS;
}
