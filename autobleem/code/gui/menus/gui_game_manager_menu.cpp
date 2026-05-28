//
// Created by screemer on 2019-01-24.
//

#include "gui_game_manager_menu.h"
#include "../../log.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <string>
#include <iostream>
#include "gui_game_editor_menu.h"
#include "../gui_confirm.h"
#include "../gui_layout.h"
#include "../../lang.h"
#include "../../launcher/thumbnail_lookup.h"
#include <ftw.h>
#include "../../engine/scanner.h"
#include "../../engine/database.h"
#include <SDL2/SDL_image.h>

using namespace std;

static const SDL_Color gameManagerHeaderColor = {80, 170, 255, 255};

namespace {

const int previewWidth = 300;
const int previewGap = 24;
const int previewRightPadding = 20;
const int previewMaxCoverHeight = 220;
const int previewCoverToScreenshotGap = 16;
const int previewMinScreenshotHeight = 30;
const int gameManagerMaxVisible = 18;
const int gameManagerFirstListRow = 4;
const int gameManagerTitleGap = 8;
const int gameManagerHeaderLineGap = 5;
const int gameManagerHeaderMinLineGap = 8;
const int gameManagerListBottomGap = 10;
const int gameManagerPanelPadding = 10;
const int gameManagerStatusClearance = 34;
const int gameManagerDefaultBottomClearance = 52;
const int gameManagerSidePadding = 10;
const int gameManagerSelectionInset = 5;
const int gameManagerFreeSpaceGap = 10;
const int gameManagerFreeSpaceBottomPadding = 6;
const int gameManagerRowToBottomLineGap = 6;
const int gameManagerHeaderRow = 2;
const int gameManagerFolderColumnX = 520;
const int gameManagerSmallFontSize = 15;
const int gameManagerMinFontSize = 12;
const int gameManagerHeaderAlpha = 95;
const int gameManagerDividerAlpha = 75;
const char localCoverExtension[] = ".png";
const char defaultCoverFile[] = "default.png";

SDL_Rect getGameManagerRect(const shared_ptr<Gui> &gui) {
    SDL_Rect rect = gui->getOpscreenRectOfTheme();
    int statusY = atoi(gui->themeData.values["ttop"].c_str());
    int maxBottom =
        statusY > 0 ? statusY - gameManagerStatusClearance : SCREEN_HEIGHT - gameManagerDefaultBottomClearance;
    if (maxBottom > rect.y + rect.h) {
        rect.h = maxBottom - rect.y;
    }
    return rect;
}

int getGameManagerContentTop(const shared_ptr<Gui> &gui) { return getGameManagerRect(gui).y + gameManagerPanelPadding; }

int getGameManagerContentBottom(const shared_ptr<Gui> &gui) {
    SDL_Rect rect = getGameManagerRect(gui);
    return rect.y + rect.h - gameManagerPanelPadding;
}

int getGameManagerFreeSpaceHeight(const shared_ptr<Gui> &gui) {
    return FC_GetLineHeight(gui->themeFonts[FONT_20_BOLD]);
}

int getGameManagerListTopY(const shared_ptr<Gui> &gui, FC_Font_Shared font) {
    int rowHeight = FC_GetLineHeight(font);
    int yoffset = getGameManagerContentTop(gui) + gameManagerTitleGap;
    return yoffset + (rowHeight * gameManagerFirstListRow);
}

int getGameManagerListBottomY(const shared_ptr<Gui> &gui) {
    return getGameManagerContentBottom(gui) - gameManagerFreeSpaceBottomPadding - getGameManagerFreeSpaceHeight(gui) -
           gameManagerFreeSpaceGap;
}

int getGameManagerRowStep(const shared_ptr<Gui> &gui, FC_Font_Shared font) {
    int rowHeight = FC_GetLineHeight(font);
    int listTop = getGameManagerListTopY(gui, font);
    int lastRowTop = getGameManagerListBottomY(gui) - rowHeight - gameManagerRowToBottomLineGap;
    if (gameManagerMaxVisible <= 1 || lastRowTop <= listTop) {
        return rowHeight;
    }

    return std::max(rowHeight, (lastRowTop - listTop) / (gameManagerMaxVisible - 1));
}

int getGameManagerRowY(const shared_ptr<Gui> &gui, FC_Font_Shared font, int row) {
    return getGameManagerListTopY(gui, font) + ((row - gameManagerFirstListRow) * getGameManagerRowStep(gui, font));
}

void renderGameManagerTextBar(const shared_ptr<Gui> &gui, SDL_Shared<SDL_Renderer> renderer) {
    string bg = gui->themeData.values["main_bg"];
    SDL_SetRenderDrawColor(renderer, gui->getR(bg), gui->getG(bg), gui->getB(bg),
                           atoi(gui->themeData.values["mainalpha"].c_str()));
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect rect = getGameManagerRect(gui);
    SDL_RenderFillRect(renderer, &rect);
}

SDL_Shared<SDL_Texture> loadPreviewCover(SDL_Shared<SDL_Renderer> renderer, const PsGamePtr &game) {
    if (!game)
        return nullptr;

    string imagePath = game->folder + sep + game->base + localCoverExtension;
    if (FileUtils::exists(imagePath))
        return IMG_LoadTexture(renderer, imagePath.c_str());

    auto gui = Gui::getInstance();
    const string raBoxArt = ThumbnailLookup::findBoxArtPath(
        ThumbnailLookup::PlayStationDbName, game->title, Coverdb::findRecordNameForSerial(gui->coverdb, game->serial));
    if (!raBoxArt.empty())
        return IMG_LoadTexture(renderer, raBoxArt.c_str());

    return IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + defaultCoverFile).c_str());
}

SDL_Shared<SDL_Texture> loadPreviewScreenshot(SDL_Shared<SDL_Renderer> renderer, const PsGamePtr &game) {
    if (!game)
        return nullptr;

    auto gui = Gui::getInstance();
    const string snapPath = ThumbnailLookup::findSnapPath(ThumbnailLookup::PlayStationDbName, game->title, game->folder,
                                                          Coverdb::findRecordNameForSerial(gui->coverdb, game->serial));
    return snapPath.empty() ? nullptr : IMG_LoadTexture(renderer, snapPath.c_str());
}

} // namespace

//*******************************
// GuiManager::init
//*******************************
void GuiManager::init() {
    useSmallerFont = true;
    xoffset_R = gameManagerFolderColumnX;
    GuiMenuBase::init(); // call the base class init()
    firstRow = gameManagerFirstListRow;
    maxVisible = gameManagerMaxVisible;
    previewIndex = -1;
    previewCover = nullptr;
    previewScreenshot = nullptr;

    lines.clear();
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
    syncVisibleWindow();
}

//*******************************
// GuiManager::render
//*******************************
void GuiManager::render() {
    SDL_RenderClear(renderer);
    gui->renderBackground();
    renderGameManagerTextBar(gui, renderer);
    yoffset = getGameManagerContentTop(gui) + gameManagerTitleGap;
    syncVisibleWindow();

    gui->renderTitleLine(getTitle(), 0, yoffset);

    renderColumnHeaders();
    renderGameRows();
    renderTableBottomLine();
    renderSelectionBox();
    renderPreview();

    int freeSpaceY = getGameManagerListBottomY(gui) + gameManagerFreeSpaceGap;
    int maxFreeSpaceY =
        getGameManagerContentBottom(gui) - getGameManagerFreeSpaceHeight(gui) - gameManagerFreeSpaceBottomPadding;
    if (freeSpaceY > maxFreeSpaceY) {
        freeSpaceY = maxFreeSpaceY;
    }
    int statusY = atoi(gui->themeData.values["ttop"].c_str());
    if (statusY > 0 && freeSpaceY > statusY - gameManagerStatusClearance) {
        freeSpaceY = statusY - gameManagerStatusClearance;
    }
    gui->renderFreeSpace(freeSpaceY); // this is why this menu's render is special instead of using the base class

    gui->renderStatus(getStatusLine());
    SDL_RenderPresent(renderer);
}

void GuiManager::syncVisibleWindow() {
    maxVisible = gameManagerMaxVisible;
    if (getVerticalSize() == 0) {
        selected = 0;
        firstVisibleIndex = 0;
        lastVisibleIndex = 0;
        firstRender = false;
        return;
    }

    if (selected < 0) {
        selected = 0;
    } else if (selected >= getVerticalSize()) {
        selected = getVerticalSize() - 1;
    }

    int expectedLastPageFirst = std::max(0, getVerticalSize() - maxVisible);
    bool lastPageWindowShort = lastVisibleIndex >= getVerticalSize() && firstVisibleIndex != expectedLastPageFirst;
    bool visibleRangeInvalid = firstRender || firstVisibleIndex < 0 || firstVisibleIndex > selected ||
                               selected > lastVisibleIndex || lastPageWindowShort ||
                               (lastVisibleIndex - firstVisibleIndex + 1) != maxVisible;
    if (visibleRangeInvalid) {
        computePagePosition();
        firstRender = false;
    }
    lastVisibleIndex = std::min(lastVisibleIndex, firstVisibleIndex + gameManagerMaxVisible - 1);
}

//*******************************
// GuiManager::getTitle
//*******************************
std::string GuiManager::getTitle() { return _("Game manager - Select game"); }

//*******************************
// GuiManager::getStatusLine
//*******************************
string GuiManager::getStatusLine() {
    int displayIndex = psGames.empty() ? 0 : selected + 1;
    return _("Game") + " " + to_string(displayIndex) + "/" + to_string(psGames.size()) + "    |@L1|/|@R1| " +
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
    if (psGames.empty()) {
        return;
    }
    auto game = psGames[selected];
    int gameId = game->gameId;
    string gameName = game->title;
    string gameSaveStateFolder = game->ssFolder;
    GuiConfirm *confirm = new GuiConfirm(renderer);
    confirm->label = _("Are you sure you want to delete") + " " + gameName + "?";
    confirm->show();
    bool delGame = confirm->result;
    delete confirm;

    if (!delGame) {
        render();
        return;
    }

    PLOG_INFO << "Trying to delete " << gameName;
    gui->renderStatus(_("Please wait... deleting") + " " + gameName);
    bool success = gui->db->deleteGameIdFromAllTables(gameId);
    bool shouldForceScan = success;
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
        } else {
            PLOG_ERROR << "Failed to delete directory " << game->folder;
            gui->renderStatus(_("Failed to delete") + " " + gameName);
        }
    } else {
        PLOG_ERROR << "Failed to delete " << gameName;
        gui->renderStatus(_("Failed to delete") + " " + gameName);
    }

    if (shouldForceScan) {
        gui->forceScan = true; // in order for the sub dir hierarchy to be fixed we have to do a rescan
    }
    // menuVisible = false;
    init(); // refresh games list and menu item count
    render();
}

void GuiManager::renderColumnHeaders() {
    SDL_Rect opscreen = getGameManagerRect(gui);
    int fontSize = useSmallerFont ? gameManagerSmallFontSize : gui->getThemeFontSize();
    int rowHeight = FC_GetLineHeight(font);
    int y = yoffset + (rowHeight * gameManagerHeaderRow);
    int previewX = opscreen.x + opscreen.w - previewWidth - previewRightPadding;
    int leftX = opscreen.x + gameManagerSidePadding + xoffset_L;
    int rightX = opscreen.x + gameManagerSidePadding + xoffset_R;
    int leftWidth = xoffset_R - xoffset_L - (gameManagerSidePadding * 2);
    int rightWidth = previewX - rightX - previewGap;

    int headerHeight = gui->renderFittedText_WithColor(font, _("Game"), leftX, y, leftWidth, fontSize,
                                                       gameManagerMinFontSize, gameManagerHeaderColor);
    headerHeight =
        std::max(headerHeight, gui->renderFittedText_WithColor(font, _("Folder"), rightX, y, rightWidth, fontSize,
                                                               gameManagerMinFontSize, gameManagerHeaderColor));

    SDL_SetRenderDrawColor(renderer, gameManagerHeaderColor.r, gameManagerHeaderColor.g, gameManagerHeaderColor.b,
                           gameManagerHeaderAlpha);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int lineY = std::min(y + headerHeight + gameManagerHeaderLineGap,
                         yoffset + (rowHeight * firstRow) - gameManagerHeaderMinLineGap);
    SDL_RenderDrawLine(renderer, opscreen.x + gameManagerSidePadding, lineY, previewX - previewGap, lineY);
}

void GuiManager::renderTableBottomLine() {
    SDL_Rect opscreen = getGameManagerRect(gui);
    int previewX = opscreen.x + opscreen.w - previewWidth - previewRightPadding;
    int lineY = getGameManagerListBottomY(gui);

    SDL_SetRenderDrawColor(renderer, gameManagerHeaderColor.r, gameManagerHeaderColor.g, gameManagerHeaderColor.b,
                           gameManagerHeaderAlpha);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawLine(renderer, opscreen.x + gameManagerSidePadding, lineY, previewX - previewGap, lineY);
}

void GuiManager::renderGameRows() {
    if (selected < 0 || getVerticalSize() == 0)
        return;

    int row = firstRow;
    int endIndex = std::min(getVerticalSize(), firstVisibleIndex + gameManagerMaxVisible);
    for (int i = firstVisibleIndex; i < endIndex; i++) {
        renderLineIndexOnRow(i, row);
        row++;
    }
}

void GuiManager::renderSelectionBox() {
    if (getVerticalSize() == 0)
        return;

    SDL_Rect opscreen = getGameManagerRect(gui);
    int previewX = opscreen.x + opscreen.w - previewWidth - previewRightPadding;
    int row = selected - firstVisibleIndex + firstRow;
    int fontHeight = FC_GetLineHeight(font);
    int selectionOffset = Gui::getSelectionBoxYOffset(font);
    SDL_Rect rectSelection;
    rectSelection.x = opscreen.x + gameManagerSelectionInset;
    rectSelection.y = getGameManagerRowY(gui, font, row);
    rectSelection.y -= selectionOffset;
    rectSelection.w = previewX - previewGap - rectSelection.x;
    rectSelection.h = fontHeight + (selectionOffset * 2);

    string fg = gui->themeData.values["text_fg"];
    SDL_Color selectionColor = {static_cast<Uint8>(gui->getR(fg)), static_cast<Uint8>(gui->getG(fg)),
                                static_cast<Uint8>(gui->getB(fg)), 255};
    GuiLayout::renderRectOutline(renderer, rectSelection, selectionColor);
}

void GuiManager::renderLineIndexOnRow(int index, int row) {
    SDL_Rect opscreen = getGameManagerRect(gui);
    int previewX = opscreen.x + opscreen.w - previewWidth - previewRightPadding;
    int menuFontSize = useSmallerFont ? gameManagerSmallFontSize : gui->getThemeFontSize();
    int leftWidth = xoffset_R - xoffset_L - (gameManagerSidePadding * 2);
    int rightWidth = previewX - (opscreen.x + gameManagerSidePadding + xoffset_R) - previewGap;
    int y = getGameManagerRowY(gui, font, row);
    int leftX = opscreen.x + gameManagerSidePadding + xoffset_L;
    int rightX = opscreen.x + gameManagerSidePadding + xoffset_R;

    gui->renderFittedText(font, lines[index].line_L, leftX, y, leftWidth, menuFontSize, gameManagerMinFontSize);
    gui->renderFittedText(font, lines[index].line_R, rightX, y, rightWidth, menuFontSize, gameManagerMinFontSize);
}

void GuiManager::updatePreviewTextures() {
    if (selected == previewIndex)
        return;

    previewIndex = selected;
    previewCover = nullptr;
    previewScreenshot = nullptr;

    if (selected < 0 || selected >= psGames.size())
        return;

    previewCover = loadPreviewCover(renderer, psGames[selected]);
    previewScreenshot = loadPreviewScreenshot(renderer, psGames[selected]);
}

void GuiManager::renderPreview() {
    updatePreviewTextures();

    SDL_Rect opscreen = getGameManagerRect(gui);
    int previewX = opscreen.x + opscreen.w - previewWidth - previewRightPadding;
    int previewTop = yoffset + FC_GetLineHeight(font);
    int previewBottom = getGameManagerContentBottom(gui);
    int previewHeight = previewBottom - previewTop;
    if (previewHeight <= 0)
        return;

    SDL_SetRenderDrawColor(renderer, gameManagerHeaderColor.r, gameManagerHeaderColor.g, gameManagerHeaderColor.b,
                           gameManagerDividerAlpha);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawLine(renderer, previewX - (previewGap / 2), previewTop, previewX - (previewGap / 2), previewBottom);

    SDL_Rect coverBounds{previewX, previewTop, previewWidth,
                         std::min(previewMaxCoverHeight, previewHeight / 2 - gameManagerHeaderMinLineGap)};
    SDL_Rect snapBounds{previewX, coverBounds.y + coverBounds.h + previewCoverToScreenshotGap, previewWidth,
                        previewBottom - (coverBounds.y + coverBounds.h + previewCoverToScreenshotGap)};

    if (previewCover != nullptr) {
        GuiLayout::renderTextureFit(renderer, previewCover, coverBounds);
    }

    if (previewScreenshot != nullptr && snapBounds.h > previewMinScreenshotHeight) {
        GuiLayout::renderTextureFit(renderer, previewScreenshot, snapBounds);
    }
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
