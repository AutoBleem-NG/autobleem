//
// Created by screemer on 2019-01-24.
//
#pragma once

#include "gui_screen.h"
#include "star_fx.h"
#include "gui_font_wrapper.h"
#include <map>
#include <string>
#include <vector>

//********************
// GuiAbout
//********************
class GuiAbout : public GuiScreen {
  public:
    StarFx fx;
    void init() override;
    void render() override;
    void loop() override;
    void renderCreditSection(const std::string &heading, const std::vector<std::string> &lines, const SDL_Rect &rect);
    SDL_Shared<SDL_Texture> logo;
    std::string bodyFontPath;
    std::string titleFontPath;
    std::string headingFontPath;
    std::map<std::string, FC_Font_Shared> fontCache;
    using GuiScreen::GuiScreen;
};
