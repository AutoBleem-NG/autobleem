#pragma once

#include "gui_font.h"
#include "gui_sdl_wrapper.h"
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Gui;

namespace GuiText {
using ColumnLine = std::pair<std::string, std::string>;

FontType getFontTypeForThemeFont(Gui *gui, FC_Font_Shared font);
std::string joinLines(const std::vector<std::string> &lines, const std::string &separator = " ");
std::string resolveThemeFontPath(const std::string &themePath, const std::string &fontName);
std::string resolveFontWithFallback(const std::string &fontName, const std::string &fallbackFontName,
                                    const std::string &themePath);
std::string resolveSelectedBodyFontPath(const std::shared_ptr<Gui> &gui);

int getLongestWordWidth(FC_Font_Shared font, const std::string &text);
int getColumnRowStep(FC_Font_Shared font);
int getColumnGroupGap(FC_Font_Shared font);
FC_Font_Shared getFittingFont(SDL_Shared<SDL_Renderer> renderer, std::map<std::string, FC_Font_Shared> &fontCache,
                              const std::string &fontPath, int maxSize, int minSize, const std::string &text, int width,
                              int height);
FC_Font_Shared getFittingColumnFont(const std::shared_ptr<Gui> &gui, const std::vector<ColumnLine> &lines, int xLeft,
                                    int xRight, int yoffset, int maxSize = 20, int minSize = 12);
void renderColumnLine(const std::shared_ptr<Gui> &gui, FC_Font_Shared font, const ColumnLine &line, int xLeft,
                      int xRight, int y);

void renderWrappedText(SDL_Shared<SDL_Renderer> renderer, FC_Font_Shared font, const std::string &text,
                       const SDL_Rect &rect, SDL_Color color);
void renderCenteredWrappedText(SDL_Shared<SDL_Renderer> renderer, FC_Font_Shared font, const std::string &text,
                               const SDL_Rect &rect, SDL_Color color);
} // namespace GuiText
