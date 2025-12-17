//
// Created by screemer on 2019-01-24.
//
#pragma once

#include "gui_screen.h"
#include "star_fx.h"
#include "gui_font_wrapper.h"

//********************
// GuiAbout
//********************
class GuiAbout : public GuiScreen {
  public:
    StarFx fx;
    void init() override;
    void render() override;
    void loop() override;
    SDL_Shared<SDL_Texture> logo;
    FC_Font_Shared font;
    using GuiScreen::GuiScreen;
};
