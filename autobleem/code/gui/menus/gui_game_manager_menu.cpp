//
// Created by screemer on 2019-01-24.
//

#include "gui_game_manager_menu.h"
#include "../../log.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <iostream>
#include "gui_game_editor_menu.h"
#include "../gui_confirm.h"
#include "../../lang.h"
#include <ftw.h>
#include "../../engine/scanner.h"
#include "../../engine/database.h"

using namespace std;

//*******************************
// GuiManager::init
//*******************************
void GuiManager::init() {
    useSmallerFont = true;
    GuiMenuBase::init(); // call the base class init()

    psGames.clear();
    gui->db->getGames(&psGames);                       // Create list of games
    sort(psGames.begin(), psGames.end(), sortByTitle); // sort by title
    for (auto &psGame : psGames) {
        // left column              right column
        // "title"                  "path"
        string path = FileUtils::removeSeparatorFromEndOfPath(psGame->folder);
        path = FileUtils::removeGamesPathFromFrontOfPath(path);
        lines.emplace_back(psGame->title, path);
    }
}

//*******************************
// GuiManager::render
//*******************************
void GuiManager::render() {
    SDL_RenderClear(renderer);
    gui->renderBackground();
    gui->renderTextBar();
    yoffset = gui->renderLogo(true);

    gui->renderFreeSpace(); // this is why this menu's render is special instead of using the base class

    gui->renderTextLine(getTitle(), 0, yoffset, XALIGN_CENTER);

    renderLines();
    renderSelectionBox();

    gui->renderStatus(getStatusLine());
    SDL_RenderPresent(renderer);
}

//*******************************
// GuiManager::getTitle
//*******************************
std::string GuiManager::getTitle() { return "-=" + _("Game manager - Select game") + "=-"; }

//*******************************
// GuiManager::getStatusLine
//*******************************
string GuiManager::getStatusLine() {
    return _("Game") + " " + to_string(selected + 1) + "/" + to_string(psGames.size()) + "    |@L1|/|@R1| " +
           _("Page") + "   |@X| " + _("Select") + "  |@S| " + _("Delete Game") + "  |@T| " + _("Flush covers") +
           " |@O| " + _("Close") + " |";
}

//*******************************
// GuiManager::flushCovers
//*******************************
int GuiManager::flushCovers(const char *file, const struct stat * /*sb*/, int /*flag*/, struct FTW * /*s*/) {
    int retval = 0;

    if (FileUtils::getFileExtension(file) == "png") {
        remove(file);
    }

    return retval;
}

//*******************************
// GuiManager::doCircle_Pressed
//*******************************
void GuiManager::doCircle_Pressed() {
    Mix_PlayChannel(-1, gui->cancel, 0);
    if (changes) {
        gui->forceScan = true;
    }
    menuVisible = false;
}

//*******************************
// GuiManager::doSquare_Pressed
//*******************************
void GuiManager::doSquare_Pressed() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    auto game = psGames[selected];
    int gameId = game->gameId;
    string gameName = game->title;
    string gameSaveStateFolder = game->ssFolder;
    GuiConfirm *confirm = new GuiConfirm(renderer);
    confirm->label = _("Are you sure you want to delete") + " " + gameName + "?";
    confirm->show();
    bool delGame = confirm->result;
    delete confirm;

    if (delGame) {
        PLOG_INFO << "Trying to delete " << gameName;
        gui->renderStatus(_("Please wait... deleting") + " " + gameName);
        bool success = gui->db->deleteGameIdFromAllTables(gameId);
        if (success) {
            success = FileUtils::removeDirAndContents(game->folder);
            if (success) {
                PsGames currentGames;
                gui->db->getGames(&currentGames);
                int numberOfGamesRemainingWithSameSaveState =
                    count_if(begin(currentGames), end(currentGames),
                             [&](const PsGamePtr &g) { return g->ssFolder == gameSaveStateFolder; });
                if (numberOfGamesRemainingWithSameSaveState == 0) {
                    GuiConfirm *confirm = new GuiConfirm(renderer);
                    confirm->label = _("Delete !SaveState folder for game") + " " + gameName + "?";
                    confirm->show();
                    bool delSSFolder = confirm->result;
                    delete confirm;
                    if (delSSFolder)
                        FileUtils::removeDirAndContents(gameSaveStateFolder);
                }
            }
        } else {
            PLOG_ERROR << "Failed to delete directory " << game->folder;
            gui->renderStatus(_("Failed to delete") + " " + gameName);
        }
    } else {
        PLOG_ERROR << "Failed to delete " << gameName;
        gui->renderStatus(_("Failed to delete") + " " + gameName);
    }
    gui->forceScan = true; // in order for the sub dir hierarchy to be fixed we have to do a rescan
    // menuVisible = false;
    init(); // refresh games list and menu item count
    render();
}

//*******************************
// GuiManager::doTriangle_Pressed
//*******************************
void GuiManager::doTriangle_Pressed() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    GuiConfirm *confirm = new GuiConfirm(renderer);
    confirm->label = _("Are you sure you want to flush all covers?");
    confirm->show();
    bool delCovers = confirm->result;
    delete confirm;

    if (delCovers) {
        PLOG_INFO << "Trying to delete covers";
        gui->renderStatus(_("Please wait... deleting covers..."));

        int errors = 0;
        int flags = FTW_DEPTH | FTW_PHYS | FTW_CHDIR;
        if (nftw(FileUtils::fixPath(gui->pathToGamesDir).c_str(), flushCovers, 1, flags) != 0) {
            errors++;
        }

        gui->forceScan = true;
        menuVisible = false;
    } else {
        render();
    }
}

//*******************************
// GuiManager::doCross_Pressed
//*******************************
void GuiManager::doCross_Pressed() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    if (!psGames.empty()) {
        string selectedGameFolder = psGames[selected]->folder;
        GuiEditor *editor = new GuiEditor(renderer);
        editor->gameData = psGames[selected];
        editor->gameFolder = selectedGameFolder;
        editor->gameIni.load(selectedGameFolder + sep + GAME_INI);
        string folderNoLast = FileUtils::removeSeparatorFromEndOfPath(selectedGameFolder);
        // change "/media/Games/Racing/Driver 2" to "Driver 2"
        editor->gameIni.entry = FileUtils::getFileNameFromPath(folderNoLast);
        editor->show();
        if (editor->changes) {
            changes = true;
        }
        selected = 0;
        firstVisibleIndex = 0;
        lastVisibleIndex = firstVisibleIndex + maxVisible - 1;

        init();
        int pos = 0;
        for (const auto &psGame : psGames) {
            if (psGame->folder == selectedGameFolder) {
                selected = pos;
                firstVisibleIndex = pos;
                lastVisibleIndex = firstVisibleIndex + maxVisible - 1;
            }
            pos++;
        }
        render();
        delete editor;
    }
}
