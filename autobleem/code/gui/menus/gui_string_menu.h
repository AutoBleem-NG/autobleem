#pragma once

#include "gui_menu_base.h"
#include <SDL2/SDL_render.h>
#include "../gui_sdl_wrapper.h"

//*******************************
// class GuiStringMenu
//*******************************
class GuiStringMenu : public GuiMenuBase<std::string> {
  public:
    explicit GuiStringMenu(SDL_Shared<SDL_Renderer> _renderer) : GuiMenuBase(_renderer) {}

    std::string getTitle() override { return GuiMenuBase::getTitle(); }
    std::string getStatusLine() override { return GuiMenuBase::getStatusLine(); }

    void renderLineIndexOnRow(int index, int row) override {
        int menuFontSize = useSmallerFont ? 15 : gui->getThemeFontSize();
        SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
        gui->renderFittedTextLine(lines[index], row, yoffset, XALIGN_LEFT, 0, opscreen.w - 20, menuFontSize, 12, font);
    }
};
