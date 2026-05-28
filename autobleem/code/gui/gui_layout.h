#pragma once

#include "gui_sdl_wrapper.h"
#include <SDL2/SDL.h>

namespace GuiLayout {
void renderPanel(SDL_Shared<SDL_Renderer> renderer, const SDL_Rect &rect, SDL_Color fill, SDL_Color border);
void renderRectOutline(SDL_Shared<SDL_Renderer> renderer, const SDL_Rect &rect, SDL_Color color);
SDL_Rect insetRect(const SDL_Rect &rect, int horizontalInset, int verticalInset);
SDL_Rect fitRectPreserveAspect(int contentWidth, int contentHeight, const SDL_Rect &bounds);
bool renderTextureFit(SDL_Shared<SDL_Renderer> renderer, SDL_Shared<SDL_Texture> texture, const SDL_Rect &bounds);
} // namespace GuiLayout
