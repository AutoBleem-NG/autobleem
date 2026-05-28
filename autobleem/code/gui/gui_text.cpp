#include "gui_text.h"
#include "gui.h"
#include "../log.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>

using namespace std;

namespace GuiText {
FontType getFontTypeForThemeFont(Gui *gui, FC_Font_Shared font) {
    if (font.font_shared_ptr == gui->themeFont.font_shared_ptr ||
        font.font_shared_ptr == gui->themeFonts[FONT_22_MED].font_shared_ptr) {
        return FONT_MED;
    }

    return FONT_BOLD;
}

string resolveSelectedBodyFontPath(const shared_ptr<Gui> &gui) {
    string themePath = gui->getCurrentThemePath() + sep;
    if (Fonts::currentLanguageNeedsCjkFont()) {
        return resolveFontWithFallback("NotoSansSC.ttf", "japanese.ttf", themePath);
    }

    string fontName = gui->themeData.values["font"];
    string configuredFontName = gui->cfg.inifile.get("font");
    if (gui->cfg.inifile.get("themefont") != "true" && configuredFontName != "" && configuredFontName != "--") {
        fontName = configuredFontName;
    }
    if (fontName == "") {
        fontName = "Inter.ttf";
    }

    return resolveFontWithFallback(fontName, "Inter.ttf", themePath);
}

int getLongestWordWidth(FC_Font_Shared font, const string &text) {
    int longest = 0;
    string word;
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (word != "") {
                longest = std::max(longest, static_cast<int>(FC_GetWidth(font, "%s", word.c_str())));
                word.clear();
            }
            continue;
        }
        word += c;
    }

    if (word != "") {
        longest = std::max(longest, static_cast<int>(FC_GetWidth(font, "%s", word.c_str())));
    }

    return longest;
}

int getColumnRowStep(FC_Font_Shared font) {
    int lineHeight = FC_GetLineHeight(font);
    return lineHeight + max(4, lineHeight / 5);
}

int getColumnGroupGap(FC_Font_Shared font) {
    int lineHeight = FC_GetLineHeight(font);
    return max(8, lineHeight / 2);
}

FC_Font_Shared getFittingFont(SDL_Shared<SDL_Renderer> renderer, map<string, FC_Font_Shared> &fontCache,
                              const string &fontPath, int maxSize, int minSize, const string &text, int width,
                              int height) {
    for (int size = maxSize; size >= minSize; --size) {
        string cacheKey = fontPath + "#" + std::to_string(size);
        if (fontCache.find(cacheKey) == fontCache.end()) {
            fontCache[cacheKey] = Fonts::openNewSharedCachedFont(fontPath, size, renderer);
        }

        FC_Font_Shared cachedFont = fontCache[cacheKey];
        if (getLongestWordWidth(cachedFont, text) <= width &&
            FC_GetColumnHeight(cachedFont, width, "%s", text.c_str()) <= height) {
            return cachedFont;
        }
    }

    string cacheKey = fontPath + "#" + std::to_string(minSize);
    if (fontCache.find(cacheKey) == fontCache.end()) {
        fontCache[cacheKey] = Fonts::openNewSharedCachedFont(fontPath, minSize, renderer);
    }
    PLOG_WARNING << "Text does not fit at minimum font size: " << text;
    return fontCache[cacheKey];
}

FC_Font_Shared getFittingColumnFont(const shared_ptr<Gui> &gui, const vector<ColumnLine> &lines, int xLeft, int xRight,
                                    int yoffset, int maxSize, int minSize) {
    int leftWidth = xRight - xLeft - 20;
    int rightWidth = SCREEN_WIDTH - xRight - 40;
    int maxHeight = SCREEN_HEIGHT - yoffset - 70;

    FC_Font_Shared baseFont = gui->themeFonts[FONT_20_BOLD];
    for (int size = maxSize; size >= minSize; --size) {
        FC_Font_Shared font = size == maxSize ? baseFont : gui->sizesOfThemeFont.GetFont(size, gui->themeFonts);
        int requiredHeight = 0;
        for (const auto &line : lines) {
            requiredHeight += line.first == "" && line.second == "" ? getColumnGroupGap(font) : getColumnRowStep(font);
        }
        bool fits = requiredHeight <= maxHeight;

        for (const auto &line : lines) {
            if (!fits) {
                break;
            }

            Gui::AllTextOrEmojiTokenInfo leftInfo(font, line.first);
            Gui::AllTextOrEmojiTokenInfo rightInfo(font, line.second);
            fits = leftInfo.totalSize.w <= leftWidth && rightInfo.totalSize.w <= rightWidth;
        }

        if (fits) {
            return font;
        }
    }

    PLOG_WARNING << "Column text does not fit at minimum font size";
    return gui->sizesOfThemeFont.GetFont(minSize, gui->themeFonts);
}

void renderColumnLine(const shared_ptr<Gui> &gui, FC_Font_Shared font, const ColumnLine &line, int xLeft, int xRight,
                      int y) {
    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    gui->renderText(font, line.first, opscreen.x + 10 + xLeft, y);
    gui->renderText(font, line.second, opscreen.x + 10 + xRight, y);
}

void renderWrappedText(SDL_Shared<SDL_Renderer> renderer, FC_Font_Shared font, const string &text, const SDL_Rect &rect,
                       SDL_Color color) {
    FC_DrawColumnColor(font, renderer, rect.x, rect.y, rect.w, color, "%s", text.c_str());
}

void renderCenteredWrappedText(SDL_Shared<SDL_Renderer> renderer, FC_Font_Shared font, const string &text,
                               const SDL_Rect &rect, SDL_Color color) {
    int textHeight = FC_GetColumnHeight(font, rect.w, "%s", text.c_str());
    int y = rect.y + std::max(0, (rect.h - textHeight) / 2);
    FC_Rect box = FC_MakeRect(rect.x, y, rect.w, rect.h);
    FC_DrawBoxEffect(font, renderer, box, FC_MakeEffect(FC_ALIGN_CENTER, FC_MakeScale(1, 1), color), "%s",
                     text.c_str());
}
} // namespace GuiText

namespace {
struct FittedLineLayout {
    FC_Font_Shared font;
    int x = 0;
    int y = 0;
};

FittedLineLayout getFittedLineLayout(Gui *gui, FC_Font_Shared font, const string &text, int line, int yoffset,
                                     int xoffset, int maxWidth, int maxSize, int minSize) {
    if (!font) {
        font = gui->themeFont;
    }

    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    Uint16 fontHeight = FC_GetLineHeight(font);
    FC_Font_Shared fittedFont = gui->getFittingThemeFont(font, maxSize, minSize, text, maxWidth);
    int x = opscreen.x + 10 + xoffset;
    int y = (fontHeight * line) + yoffset;

    if (line < 0) {
        y = -line;
    }

    FittedLineLayout layout;
    layout.font = fittedFont;
    layout.x = x;
    layout.y = y;
    return layout;
}
} // namespace

int Gui::getFontPointSize(FontEnum fontEnum) {
    switch (fontEnum) {
    case FONT_15_BOLD:
        return 15;
    case FONT_20_BOLD:
        return 20;
    case FONT_22_MED:
        return 22;
    case FONT_28_BOLD:
        return 28;
    }

    return 20;
}

int Gui::getTextVisualYOffset(FC_Font_Shared /*font*/) { return 0; }

int Gui::getSelectionBoxYOffset(FC_Font_Shared font) {
    return std::max(1, static_cast<int>(FC_GetLineHeight(font) / 10));
}

int Gui::getEmojiVisualYOffset(const std::string &tokenString) {
    if (tokenString == "@L1" || tokenString == "@R1" || tokenString == "@L2" || tokenString == "@R2") {
        return 2;
    }

    return 0;
}

SDL_Color Gui::getTitleTextColor() { return {80, 170, 255, 255}; }

int Gui::getThemeFontSize(int fallback) {
    int fontSize = atoi(themeData.values["fsize"].c_str());
    return fontSize > 0 ? fontSize : fallback;
}

int Gui::getTitleFontSize() { return std::min(getThemeFontSize(), 20); }

int Gui::renderTitleLine(const string &text, int line, int yoffset, FC_Font_Shared font) {
    if (!font) {
        font = themeFonts[FONT_20_BOLD];
    }

    return renderFittedTextLine_WithColor(text, line, yoffset, XALIGN_CENTER, 0, SCREEN_WIDTH - 80, getTitleFontSize(),
                                          12, getTitleTextColor(), font);
}

FC_Font_Shared Gui::getFittingThemeFont(FC_Font_Shared baseFont, int maxSize, int minSize, const string &text,
                                        int maxWidth) {
    if (!baseFont) {
        baseFont = themeFonts[FONT_20_BOLD];
    }
    FontType fontType = GuiText::getFontTypeForThemeFont(this, baseFont);

    for (int size = maxSize; size >= minSize; --size) {
        FC_Font_Shared font = size == maxSize ? baseFont : sizesOfThemeFont.GetFont(size, themeFonts, fontType);
        AllTextOrEmojiTokenInfo tokenInfo(font, text);
        if (tokenInfo.totalSize.w <= maxWidth) {
            return font;
        }
    }

    PLOG_WARNING << "UI text does not fit at minimum font size: " << text;
    return sizesOfThemeFont.GetFont(minSize, themeFonts, fontType);
}

int Gui::renderFittedText(FC_Font_Shared baseFont, const string &text, int x, int y, int maxWidth, int maxSize,
                          int minSize, XAlignment xAlign) {
    FC_Font_Shared font = getFittingThemeFont(baseFont, maxSize, minSize, text, maxWidth);
    return renderText(font, text, x, y, xAlign);
}

int Gui::renderFittedText_WithColor(FC_Font_Shared baseFont, const string &text, int x, int y, int maxWidth,
                                    int maxSize, int minSize, SDL_Color textColor, XAlignment xAlign, bool background) {
    FC_Font_Shared font = getFittingThemeFont(baseFont, maxSize, minSize, text, maxWidth);
    return renderText_WithColor(font, text, x, y, textColor, xAlign, background);
}

int Gui::renderFittedTextLine(const string &text, int line, int yoffset, XAlignment xAlign, int xoffset, int maxWidth,
                              int maxSize, int minSize, FC_Font_Shared font) {
    FittedLineLayout layout = getFittedLineLayout(this, font, text, line, yoffset, xoffset, maxWidth, maxSize, minSize);
    return renderText(layout.font, text, layout.x, layout.y, xAlign);
}

int Gui::renderFittedTextLine_WithColor(const string &text, int line, int yoffset, XAlignment xAlign, int xoffset,
                                        int maxWidth, int maxSize, int minSize, SDL_Color textColor,
                                        FC_Font_Shared font) {
    FittedLineLayout layout = getFittedLineLayout(this, font, text, line, yoffset, xoffset, maxWidth, maxSize, minSize);
    return renderText_WithColor(layout.font, text, layout.x, layout.y, textColor, xAlign);
}
