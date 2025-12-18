//
// Created by screemer on 2/13/19.
//

#include <sys/wait.h>
#include "pcsx_interceptor.h"
#include "../utils/string_utils.h"
#include "../utils/time_utils.h"
#include "../gui/gui.h"
#include "../lang.h"
#include "../engine/mem_card.h"
#include "../engine/cfg_processor.h"
#include <fstream>
#include <iostream>
#include "../log.h"
#include <unistd.h>
#include "../environment.h"

using namespace std;

void PcsxInterceptor::cleanupConfig(PsGamePtr &game) {
    // copy back config to its place
    auto processor = new CfgProcessor();
    string newConfig = game->ssFolder + sep + "autobleem.cfg";
    if (FileUtils::exists(newConfig)) {
        // fix bios
        processor->replaceRaConf(newConfig, "Bios", "Bios = SET_BY_PCSX");

        if (!game->internal) {
            FileUtils::copy(newConfig, game->ssFolder + sep + PCSX_CFG);
            FileUtils::copy(newConfig, game->folder + sep + PCSX_CFG);
        } else {
            FileUtils::copy(newConfig, game->ssFolder + sep + PCSX_CFG);
        }
        FileUtils::removeFile(newConfig);
    }
    delete processor;
}
//*******************************
// PcsxInterceptor::execute
//*******************************
bool PcsxInterceptor::execute(PsGamePtr &game, int resumepoint) {
    PLOG_DEBUG << "calling PcsxInterceptor::execute()";

    shared_ptr<Gui> gui(Gui::getInstance());

    if (game->internal) {
        gui->internalDB->updateDatePlayed(game->gameId, TimeUtils::getCurrentTime());
    } else {
        gui->db->updateDatePlayed(game->gameId, TimeUtils::getCurrentTime());
    }

    string padMapping = gui->padMapping;

    string lastCDpoint = game->ssFolder + sep + "lastcdimg.txt";
    string lastCDpointX = game->ssFolder + sep + "lastcdimg." + to_string(resumepoint) + ".txt";
    gui->saveSelection();
    std::vector<const char *> argvNew;
    string gameFile = "";

    string region = "2"; // need to find out if console is jap to switch to 2 - later on
    string link = "/media/Autobleem/rc/launch.sh";
    string aspect = "0";
    if (gui->cfg.inifile.values["aspect"] == "true") {
        aspect = "1";
    }

    string filter = "0";
    if (gui->cfg.inifile.values["mip"] == "true") {
        filter = "1";
    } else {
        filter = "0";
    }

    trim(game->ssFolder);
    game->ssFolder = FileUtils::removeSeparatorFromEndOfPath(game->ssFolder);

    argvNew.push_back(link.c_str());
    argvNew.push_back(game->ssFolder.c_str());

    remove(lastCDpoint.c_str());

    if (FileUtils::exists(lastCDpointX)) {
        FileUtils::copy(lastCDpointX, lastCDpoint);
        ifstream is(lastCDpointX.c_str());
        if (is.is_open()) {
            std::string line;
            std::getline(is, line);

            // last line is our filename
            gameFile = line;
            is.close();
        }
    } else {
        gameFile += (game->folder + sep + game->base);
        if (!(FileUtils::matchExtension(game->base, ".pbp") || FileUtils::matchExtension(game->base, ".chd"))) {
            gameFile += ".cue";
        }
    }

    gameFile += "";

    argvNew.push_back(gameFile.c_str());
    // hack to get language from lang file
    string langStr = _("|@lang|");
    if (langStr == "|@lang|") {
        langStr = "2";
    }
    argvNew.push_back(langStr.c_str()); // lang by language file hack
    argvNew.push_back(region.c_str());
    argvNew.push_back(game->folder.c_str());
    if (resumepoint != -1) {
        argvNew.push_back("1");
    } else {
        argvNew.push_back("0");
    }
    argvNew.push_back(aspect.c_str());
    argvNew.push_back(filter.c_str());

    if (padMapping.empty()) {
        argvNew.push_back("NA");
    } else {
        argvNew.push_back(padMapping.c_str());
    }

    argvNew.push_back(nullptr);

    PLOG_DEBUG << "CMD line to execute: ";
    for (const char *s : argvNew) {
        if (s != nullptr) {
            PLOG_DEBUG << "'" << s << "' ";
        }
    }
    PLOG_DEBUG << endl;

#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    Gui::splash("I'm sorry Dave.  I'm afraid I can't do that.");
#else
    int pid = fork();
    if (!pid) {
        execvp(link.c_str(), (char **)argvNew.data());
    }

    waitpid(pid, NULL, 0);
#endif
    cleanupConfig(game);

    usleep(3 * 1000);
    return true;
}

//*******************************
// PcsxInterceptor::memcardIn
//*******************************
void PcsxInterceptor::memcardIn(PsGamePtr &game) {
    string memcard = "SONY";
    if (!game->internal) {
        Inifile gameini;
        gameini.load(game->folder + sep + GAME_INI);
        memcard = gameini.values["memcard"];
    }
    if (memcard != "SONY") {
        if (FileUtils::exists(Env::getPathToMemCardsDir() + sep + game->memcard)) {
            PLOG_DEBUG << "Swapping in memcard: " << game->memcard;
            Memcard *card = new Memcard(Env::getPathToGamesDir() + sep);
            if (!card->swapIn(game->ssFolder, game->memcard)) {
                PLOG_WARNING << "Failed to swap in memcard, reverting to SONY";
                game->setMemCard("SONY");
            };
            delete card;
        }
    }
}

//*******************************
// PcsxInterceptor::memcardOut
//*******************************
void PcsxInterceptor::memcardOut(PsGamePtr &game) {
    string memcard = "SONY";
    if (!game->internal) {
        Inifile gameini;
        gameini.load(game->folder + sep + GAME_INI);
        memcard = gameini.values["memcard"];
    }
    if (memcard != "SONY") {
        Memcard *card = new Memcard(Env::getPathToGamesDir() + sep);
        card->swapOut(game->ssFolder, game->memcard);
        delete card;
    }
}

//*******************************
// PcsxInterceptor::saveResumePoint
//*******************************
void PcsxInterceptor::saveResumePoint(PsGamePtr &game, int pointId) {
    string filenamefile = game->ssFolder + sep + "filename.txt";
    string filenamefileX = game->ssFolder + sep + "filename.txt.res";
    string filenamepoint = game->ssFolder + sep + "filename." + to_string(pointId) + ".txt.res";
    string lastCDpoint = game->ssFolder + sep + "lastcdimg.txt";
    string lastCDpointX = game->ssFolder + sep + "lastcdimg." + to_string(pointId) + ".txt";
    if (FileUtils::exists(filenamefile)) {
        ifstream is(filenamefile.c_str());
        if (is.is_open()) {

            std::string line;
            std::getline(is, line);
            std::getline(is, line);

            // last line is our filename
            string ssfile = game->ssFolder + sep + "sstates/" + line + ".00" + to_string(pointId) + ".res";
            string newName = game->ssFolder + sep + "sstates/" + line + ".000";

            FileUtils::removeFile(ssfile);
            FileUtils::copy(newName.c_str(), ssfile.c_str());
            FileUtils::removeFile(newName);

            // update image

            is.close();
        }
        FileUtils::removeFile(filenamefileX);
        FileUtils::removeFile(filenamepoint);
        FileUtils::renameFile(filenamefile, filenamefileX);
        FileUtils::copy(filenamefileX, filenamepoint);
        if (FileUtils::exists(lastCDpoint)) {
            FileUtils::removeFile(lastCDpointX);
            FileUtils::copy(lastCDpoint, lastCDpointX);
            FileUtils::removeFile(lastCDpoint);
        }
    }
}

//*******************************
// PcsxInterceptor::prepareResumePoint
//*******************************
void PcsxInterceptor::prepareResumePoint(PsGamePtr &game, int pointId) {

    // cleanup after previous crash as pcsx doest not want to save
    string filenameTrash = game->ssFolder + sep + "filename.txt";
    if (FileUtils::exists(filenameTrash)) {
        FileUtils::removeFile(filenameTrash);
    }

    string ssfile = game->ssFolder + sep + "sstates";
    for (const DirEntry &sstate : FileUtils::diru(ssfile)) {
        if (FileUtils::getFileExtension(sstate.name) == "000") {
            string toDelete = ssfile + sep + sstate.name;
            FileUtils::removeFile(toDelete);
        }
    }

    ssfile = game->ssFolder + sep + "screenshots";
    for (const DirEntry &sstate : FileUtils::diru(ssfile)) {
        if (FileUtils::getFileExtension(sstate.name) == "png") {
            string toDelete = ssfile + sep + sstate.name;
            FileUtils::removeFile(toDelete);
        }
    }

    if (pointId == -1)
        return;
    string filenamefile = game->ssFolder + sep + "filename.txt.res";
    string filenamefileX = game->ssFolder + sep + "filename.txt";
    string filenamepoint = game->ssFolder + sep + "filename." + to_string(pointId) + ".txt.res";
    FileUtils::removeFile(filenamefileX);
    if (FileUtils::exists(filenamepoint)) {
        filenamefile = filenamepoint;
    }
    if (FileUtils::exists(filenamefile)) {
        ifstream is(filenamefile.c_str());
        if (is.is_open()) {

            std::string line;
            std::getline(is, line);
            string lastImageInfo = line;

            // fix lastcdpoint
            string lastCDpointX = game->ssFolder + sep + "lastcdimg." + to_string(pointId) + ".txt";
            FileUtils::removeFile(lastCDpointX);
            string file = FileUtils::getFileNameFromPath(lastImageInfo);
            string imageToLoad = game->folder + sep + file;

            ofstream os;
            os.open(lastCDpointX);
            os << imageToLoad << endl;
            os.close();

            std::getline(is, line);

            // last line is our filename
            string ssfile = game->ssFolder + sep + "sstates/" + line + ".00" + to_string(pointId) + ".res";
            string newName = game->ssFolder + sep + "sstates/" + line + ".000";
            if (FileUtils::exists(ssfile)) {
                FileUtils::removeFile(newName);
                FileUtils::copy(ssfile.c_str(), newName.c_str());
            }
            is.close();
        }
    }
}
