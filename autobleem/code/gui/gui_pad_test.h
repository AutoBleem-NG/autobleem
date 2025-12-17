#pragma once

#include "gui_scroll_win.h"

//******************
// GuiPadTest
//******************
class GuiPadTest : public GuiScrollWin {
  public:
    void init() override;
    void render() override { GuiScrollWin::render(); }
    void loop() override;

    SDL_JoystickID joyid = -1;

    explicit GuiPadTest(SDL_Shared<SDL_Renderer> renderer1) : GuiScrollWin(renderer1) {};
};
