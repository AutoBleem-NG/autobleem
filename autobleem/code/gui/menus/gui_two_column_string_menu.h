#pragma once

#include "gui_menu_base.h"

//*******************************
// struct TwoColumnsOfText
//*******************************
struct TwoColumnsOfText {
    std::string line_L;
    std::string line_R;
    TwoColumnsOfText(std::string left, std::string right) : line_L(left), line_R(right) {}
};

//*******************************
// class GuiTwoColumnStringMenu
//*******************************
class GuiTwoColumnStringMenu : public GuiMenuBase<TwoColumnsOfText> {
  public:
    explicit GuiTwoColumnStringMenu(SDL_Shared<SDL_Renderer> _renderer) : GuiMenuBase(_renderer) {}

    int xoffset_L = 0;
    int xoffset_R = 500;

    std::string getTitle() override { return GuiMenuBase::getTitle(); }
    std::string getStatusLine() override { return GuiMenuBase::getStatusLine(); }

    void renderLineIndexOnRow(int index, int row) override {
        int menuFontSize = useSmallerFont ? 15 : gui->getThemeFontSize();
        gui->renderFittedTextLine(lines[index].line_L, row, yoffset, XALIGN_LEFT, xoffset_L, xoffset_R - xoffset_L - 20,
                                  menuFontSize, 12, font);
        gui->renderFittedTextLine(lines[index].line_R, row, yoffset, XALIGN_LEFT, xoffset_R,
                                  SCREEN_WIDTH - xoffset_R - 40, menuFontSize, 12, font);
    }
};
