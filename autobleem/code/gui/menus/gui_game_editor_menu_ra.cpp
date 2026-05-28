//
// Created by steve on 3/27/22.
//

#include "gui_game_editor_menu_ra.h"
#include "../../log.h"
#include "../gui.h"
#include "../gui_layout.h"
#include "../gui_keyboard.h"
#include "../gui_select_mem_card.h"
#include "../../engine/mem_card.h"
#include "../../engine/cfg_processor.h"
#include <SDL2/SDL_image.h>
#include "../../lang.h"
#include <sstream>
#include "../../environment.h"
#include "../../lightgun_games.h"
#include "../../system/process_utils.h"
#include <iostream>
#include "../../launcher/ra_integrator.h"
#include "../../launcher/thumbnail_lookup.h"

using namespace std;

#define OPT_FIRST 5
#define OPT_LIGHTGUN 5
#define OPT_LAST 5

//*******************************
// GuiEditor_RA::processOptionChange
//*******************************
void GuiEditor_RA::processOptionChange(bool direction) {
    shared_ptr<Gui> gui(Gui::getInstance());

    stringstream ss;
    string s;

    switch (selOption) {
    case OPT_LIGHTGUN: {
        auto &lightgunGames = Gui::getInstance()->lightgunGames;
        bool isLightgun = lightgunGames.IsGameALightgunGame(gameData);

        if (direction == true) {
            if (isLightgun == false) {
                lightgunGames.AddGame(gameData);
                isLightgun = true;
            }
        } else {
            if (isLightgun == true) {
                lightgunGames.RemoveGame(gameData);
                isLightgun = false;
            }
        }
    } break;
    }
}

//*******************************
// GuiEditor_RA::refreshData
//*******************************
void GuiEditor_RA::refreshData() { shared_ptr<Gui> gui(Gui::getInstance()); }

//*******************************
// GuiEditor_RA::GetBoxArtTexture
//*******************************
SDL_Shared<SDL_Texture> GuiEditor_RA::GetBoxArtTexture() {
    auto gui = Gui::getInstance();
    string recordName = Coverdb::findRecordNameForSerial(gui->coverdb, gameData->serial);

    SDL_Shared<SDL_Texture> coverPng;
    const string imagePath = ThumbnailLookup::findBoxArtPath(gameData->db_name, gameData->title, recordName);
    if (!imagePath.empty()) {
        coverPng = IMG_LoadTexture(renderer, imagePath.c_str());
    } else {
        PLOG_DEBUG << "boxart image NOT found for " << gameData->title;
        coverPng = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "evoimg/ra-cover.png").c_str());
    }
    return coverPng;
}

//*******************************
// GuiEditor_RA::init
//*******************************
void GuiEditor_RA::init() {
    shared_ptr<Gui> gui(Gui::getInstance());

    cover = GetBoxArtTexture();

    refreshData();
}

//*******************************
// GuiEditor_RA::render
//*******************************
void GuiEditor_RA::render() {
    shared_ptr<Gui> gui(Gui::getInstance());

    int line = 0;
    gui->renderBackground();
    gui->renderTextBar();
    int yoffset = gui->getContentTopY();

    // Game.ini
    auto editorFont = gui->themeFonts[FONT_22_MED];
    const int editorTextWidth = SCREEN_WIDTH - 80;
    gui->renderTitleLine(gameData->title, line++, yoffset);
    gui->renderFittedTextLine(_("Game File") + ": " + gameData->image_path, line++, yoffset, XALIGN_CENTER, 0,
                              editorTextWidth, 22, 12, editorFont);
    gui->renderFittedTextLine(_("Game Core") + ": " + gameData->core_name, line++, yoffset, XALIGN_CENTER, 0,
                              editorTextWidth, 22, 12, editorFont);

    gui->renderTextLineOptions(_("Lightgun Game") + ":" +
                                   (Gui::getInstance()->lightgunGames.IsGameALightgunGame(gameData)
                                        ? string("|@Check|")
                                        : string("|@Uncheck|")),
                               OPT_LIGHTGUN, yoffset, XALIGN_LEFT, 300);

    gui->renderSelectionBox(selOption, yoffset, 300);

    // string guiMenu = "|@T| " + _("Rename");

    string guiMenu = " |@O| " + _("Go back") + "|";

    gui->renderStatus(guiMenu);

    // ******************************************************************************
    // display RA cover here
    // ******************************************************************************

    SDL_Rect coverBounds{atoi(gui->themeData.values["ecoverx"].c_str()), atoi(gui->themeData.values["ecovery"].c_str()),
                         226, 226};
    GuiLayout::renderTextureFit(renderer, cover, coverBounds);

    SDL_RenderPresent(renderer);
}

//*******************************
// GuiEditor_RA::loop
//*******************************
void GuiEditor_RA::loop() {
    shared_ptr<Gui> gui(Gui::getInstance());
    menuVisible = true;
    while (menuVisible) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            gui->mapper.handleHotPlug(&e);
            gui->mapper.handlePowerBtn(&e);
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.scancode == SDL_SCANCODE_SLEEP || e.key.keysym.sym == SDLK_ESCAPE) {
                    gui->drawText(_("POWERING OFF... PLEASE WAIT"));
                    System::shutdown();
                }
            }
            // this is for pc Only
            if (e.type == SDL_QUIT) {
                menuVisible = false;
            }
            switch (e.type) {
            case SDL_CONTROLLERHATMOTIONDOWN: /* Handle Joystick Motion */
            case SDL_CONTROLLERHATMOTIONUP:

                if (gui->mapper.isDown(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        selOption++;
                        if (selOption > OPT_LAST) {
                            selOption = OPT_LAST;
                        }
                        render();
                    } while (fastForwardUntilAnotherEvent(120));
                }
                if (gui->mapper.isUp(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        selOption--;
                        if (selOption < OPT_FIRST) {
                            selOption = OPT_FIRST;
                        }
                        render();
                    } while (fastForwardUntilAnotherEvent(120));
                }

                if (gui->mapper.isRight(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        processOptionChange(true);
                        render();
                    } while (fastForwardUntilAnotherEvent(80));
                }
                if (gui->mapper.isLeft(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        processOptionChange(false);
                        render();
                    } while (fastForwardUntilAnotherEvent(80));
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    cover = nullptr;
                    menuVisible = false;
                };
            }
        }
        render();
    }
}
