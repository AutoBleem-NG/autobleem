//
// Created by screemer on 2019-01-24.
//

#include "gui_confirm.h"
#include "gui_about.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "gui.h"
#include "../lang.h"
#include "../engine/scanner.h"
#include "../system/process_utils.h"
using namespace std;

//*******************************
// GuiConfirm::render
//*******************************
void GuiConfirm::render() {
    shared_ptr<Gui> gui(Gui::getInstance());
    gui->renderBackground();
    gui->renderTextBar();
    int yoffset = gui->getContentTopY();
    int fontSize = gui->getThemeFontSize();
    gui->renderTitleLine(_("Please confirm"), 0, yoffset);
    gui->renderFittedTextLine(label, 2, yoffset, XALIGN_CENTER, 0, SCREEN_WIDTH - 80, fontSize, 12);

    gui->renderStatus("|@X| " + _("Confirm") + "  |@O| " + _("Cancel") + " |");
    SDL_RenderPresent(renderer);
}

//*******************************
// GuiConfirm::loop
//*******************************
void GuiConfirm::loop() {
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
            case SDL_CONTROLLERBUTTONDOWN:
                if (e.cbutton.button == SDL_BTN_CROSS) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    result = true;
                    menuVisible = false;
                };

                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    result = false;
                    menuVisible = false;
                };
                break;

            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_RETURN) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    result = true;
                    menuVisible = false;
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    result = false;
                    menuVisible = false;
                }
                break;
            }
        }
    }
}
