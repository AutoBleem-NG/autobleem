#pragma once

#include "gui_font_wrapper.h"
#include "gui_sdl_wrapper.h"
#include <map>
#include <string>
#include <utility>

enum FontEnum {
    FONT_15_BOLD,
    FONT_20_BOLD,
    FONT_22_MED,
    FONT_28_BOLD,
};
enum FontType { FONT_MED, FONT_BOLD };

//********************
// Fonts
//********************
class Fonts {
    std::string rootPath;
    std::string medPath;
    std::string boldPath;

    struct FontInfo {
        FontEnum fontEnum;
        int size;
        FontType fontType;
    };

    static FontInfo allFontInfos[];

    std::map<FontEnum, FC_Font_Shared> fonts;
    std::map<FontEnum, FontInfo> fontInfos;

  public:
    Fonts();

    // use operator [] to get or set the shared font
    FC_Font_Shared &operator[](FontEnum size) { return fonts[size]; }

    static FC_Font_Shared openNewSharedCachedFont(const std::string &filename, int fontSize,
                                                  SDL_Shared<SDL_Renderer> renderer);
    FC_Font_Shared openSpecificSharedCachedFont(FontType type, int fontSize) const;
    static bool currentLanguageNeedsCjkFont();
    static std::string getThemeFontPath(const std::string &themePath, const std::string &fontName);
    static std::string getRetroarchFontPath(const std::string &fontName);
    static std::string getResourceFontPath(const std::string &rootPath, const std::string &fontName);
    static std::string getFirstAvailableFontPath(const std::string &path);
    static std::string getFontPathForCurrentLanguage(const std::string &rootPath, FontType type);

    // static TTF_Font_Shared openNewSharedTTFFont(const std::string &filename, int fontSize);

    // in gui_launcher.cpp this call is used to change all the fonts to use the fonts in the current theme
    void openAllFonts(const std::string &_rootPath, SDL_Shared<SDL_Renderer> renderer);
    void openAllFontsFromFontFile(const std::string &fontPath, SDL_Shared<SDL_Renderer> renderer);
};

//********************
// SizesOfThemeFont
// Caches resized theme fonts for text that needs to shrink to fit its available width.
//********************
class SizesOfThemeFont {
    std::map<std::pair<FontType, int>, FC_Font_Shared> fonts;

  public:
    SizesOfThemeFont() = default;
    void Init() { fonts.clear(); }
    FC_Font_Shared AddFont(int size, FC_Font_Shared font, FontType type = FONT_BOLD);
    FC_Font_Shared GetFont(int size, const Fonts &fonts, FontType type = FONT_BOLD);
};

using FC_Point = SDL_Point;
struct FC_Size {
    int w = 0, h = 0;
};
