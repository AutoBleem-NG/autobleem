//
// Created by screemer on 2019-12-04.
//

#include "gui_app_start.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <sstream>
#include <iostream>
#include "../log.h"
#include "../gui/gui.h"
#include "../lang.h"
#include "../engine/scanner.h"
#include "../system/process_utils.h"

using namespace std;

std::string GuiAppStart::getStringLine(const std::string &str, int lineNo) {
    std::string line;
    std::istringstream stream(str);
    while (lineNo-- >= 0)
        std::getline(stream, line);
    return line;
}

void GuiAppStart::init() {
    shared_ptr<Gui> gui(Gui::getInstance());
    font = gui->themeFonts[FONT_20_BOLD];
    // Try to load app.ini
    appName = game->title;

    if (FileUtils::exists(game->readme_path)) {
        std::ifstream t(game->readme_path);
        t.seekg(0, std::ios::end);
        size_t size = t.tellg();
        PLOG_DEBUG << "Readme file size:" << size;
        buffer = "";
        t.seekg(0);
        std::string temp;
        while (std::getline(t, temp)) {
            buffer = buffer + temp + "\n";
        };

        int newlines = 0;
        const char *p = &buffer.c_str()[0];
        for (int i = 0; i < size; i++) {
            if (p[i] == '\n') {
                newlines++;
            }
        }
        totalLines = newlines + 1;
        readmeLoaded = true;
    }
}

void GuiAppStart::render() {
    shared_ptr<Gui> gui(Gui::getInstance());
    gui->renderBackground();
    // readme:
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect rect2;
    rect2.x = 10;
    rect2.y = 10;
    rect2.w = 1260;
    rect2.h = 600;

    SDL_RenderFillRect(renderer, &rect2);

    // scrollbar
    rect2.x = 1240;
    rect2.y = 40;
    rect2.w = 20;
    rect2.h = 20 * 25;

    SDL_RenderFillRect(renderer, &rect2);

    // draw scroll position
    if (maxLines < totalLines) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        int heightOfBar = 500 / (totalLines - maxLines);

        rect2.x = 1242;
        rect2.y = 40 + firstLine * heightOfBar;
        rect2.w = 16;
        rect2.h = heightOfBar;
        SDL_RenderFillRect(renderer, &rect2);
    }
    int yoffset = 15;
    int x = gui->getOpscreenRectOfTheme().x + 20;
    int lineHeight = FC_GetLineHeight(font);
    gui->renderFittedText(font, appName, x, yoffset, 1200, 20, 12);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, rect2.x, 35, rect2.w, 35);

    if (scrolling > 0) {
        firstLine++;
    }
    if (scrolling < 0) {
        firstLine--;
    }
    if (firstLine < 0) {
        firstLine = 0;
    }
    if (maxLines < totalLines) {
        if (firstLine > totalLines - maxLines) {
            firstLine = totalLines - maxLines;
        }
    } else {
        if (firstLine > totalLines - maxLines) {
            firstLine = 0;
        }
    }

    int currentLine = 2;
    if (!readmeLoaded) {
        gui->renderFittedText(font, _("ReadMe file not found"), x, yoffset + currentLine * lineHeight, 1200, 20, 12);
    } else {
        for (int i = firstLine; i < firstLine + maxLines; i++) {
            std::string lineInFile = getStringLine(buffer, i);
            gui->renderFittedText(font, lineInFile, x, yoffset + currentLine * lineHeight, 1200, 20, 12);
            currentLine++;
        }
    }

    gui->renderStatus("|@X| " + _("OK") + "  |@O| " + _("Cancel") + "|");
    SDL_RenderPresent(renderer);
}

void GuiAppStart::loop() {
    shared_ptr<Gui> gui(Gui::getInstance());
    bool menuVisible = true;
    while (menuVisible) {
        SDL_Event e;
        render();
        while (SDL_PollEvent(&e)) {
            gui->mapper.handleHotPlug(&e);
            gui->mapper.handlePowerBtn(&e);
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.scancode == SDL_SCANCODE_SLEEP || e.key.keysym.sym == SDLK_ESCAPE) {
                    gui->drawText(_("POWERING OFF... PLEASE WAIT"));
                    System::shutdown();
                }
            }

            switch (e.type) {
            case SDL_CONTROLLERBUTTONDOWN:
                if (e.cbutton.button == SDL_BTN_CROSS) {
                    result = true;
                    menuVisible = false;
                };
                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    result = false;
                    menuVisible = false;
                };
                break;

            case SDL_CONTROLLERHATMOTIONDOWN: /* Handle Joystick Motion */
            case SDL_CONTROLLERHATMOTIONUP:
                if (totalLines != 0) {
                    if (gui->mapper.isUp(&e)) {
                        scrolling = -1;
                    }
                    if (gui->mapper.isDown(&e)) {

                        scrolling = 1;
                    }
                    if (gui->mapper.isCenter(&e)) {
                        scrolling = 0;
                    }
                }
                break;
            }
        }
    }
}
