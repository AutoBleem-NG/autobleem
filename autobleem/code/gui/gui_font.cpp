#include "gui_font.h"
#include <iostream>
#include "../log.h"
#include "../lang.h"
#include <cassert>
#include "../utils/file_utils.h"
#include "gui.h"

using namespace std;

//********************
// static Fonts::allFontInfos
//********************
Fonts::FontInfo Fonts::allFontInfos[] = {{FONT_15_BOLD, 15, FONT_BOLD},
                                         {FONT_20_BOLD, 20, FONT_BOLD},
                                         {FONT_22_MED, 22, FONT_MED},
                                         {FONT_28_BOLD, 28, FONT_BOLD}};

//********************
// Fonts::Fonts
//********************
Fonts::Fonts() = default;

//********************
// Fonts::openNewSharedCachedFont
// low level open shared font.  filename is the full path to the ttf file.  fontSize is the font point size.
//********************
FC_Font_Shared Fonts::openNewSharedCachedFont(const string &filename, int fontSize, SDL_Shared<SDL_Renderer> renderer) {
    FC_Font *fc_font = FC_CreateFont();
    FC_LoadFont(fc_font, renderer, filename.c_str(), fontSize, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);
    FC_Font_Shared font = FC_Font_Shared(fc_font);
    if (font) {
        // PLOG_DEBUG << "Success opening font " << filename << " of size " << fontSize ;
    } else {
        PLOG_DEBUG << "FAILURE opening font " << filename << " of size " << fontSize;
        font = nullptr;
        assert(false);
    }

    return font;
}

//********************
// Fonts::openSpecificSharedCachedFont
// low level open shared font.  type is the font type (FONT_MED, FONT_BOLD).  fontSize is the font point size.
//********************
FC_Font_Shared Fonts::openSpecificSharedCachedFont(FontType type, int fontSize) {
    auto gui = Gui::getInstance();
    auto renderer = gui->renderer;

    string rootPath = gui->getCurrentThemeFontPath();
    string fontPath = getFontPathForCurrentLanguage(rootPath, type);

    FC_Font *fc_font = FC_CreateFont();
    FC_LoadFont(fc_font, renderer, fontPath.c_str(), fontSize, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);
    FC_Font_Shared font = FC_Font_Shared(fc_font);
    if (font) {
        // PLOG_DEBUG << "Success opening font " << fontPath << " of size " << fontSize ;
    } else {
        PLOG_DEBUG << "FAILURE opening font " << fontPath << " of size " << fontSize;
        font = nullptr;
        assert(false);
    }

    return font;
}

//********************
// Fonts::currentLanguageNeedsCjkFont
//********************
bool Fonts::currentLanguageNeedsCjkFont() {
    auto lang = Lang::getInstance();
    return lang->currentLang.find("Chinese") != string::npos;
}

//********************
// Fonts::getResourceFontPath
//********************
string Fonts::getResourceFontPath(const string &rootPath, const string &fontName) {
    string fontsDirPath = rootPath + sep + "fonts" + sep + fontName;
    if (FileUtils::exists(fontsDirPath)) {
        return fontsDirPath;
    }

    string legacyFontDirPath = rootPath + sep + "font" + sep + fontName;
    if (FileUtils::exists(legacyFontDirPath)) {
        return legacyFontDirPath;
    }

    return rootPath + sep + fontName;
}

//********************
// Fonts::getFontPathForCurrentLanguage
//********************
string Fonts::getFontPathForCurrentLanguage(const string &rootPath, FontType type) {
    string defaultPath;
    if (type == FONT_MED)
        defaultPath = rootPath + sep + "SST-Medium.ttf";
    else
        defaultPath = rootPath + sep + "SST-Bold.ttf";

    if (!currentLanguageNeedsCjkFont()) {
        return defaultPath;
    }

    string cjkPath;
    if (type == FONT_MED)
        cjkPath = rootPath + sep + "SSTJapanese-Regular.ttf";
    else
        cjkPath = rootPath + sep + "SSTJapanese-Bold.ttf";

    if (FileUtils::exists(cjkPath)) {
        return cjkPath;
    }

    string regularCjkPath = rootPath + sep + "SSTJapanese-Regular.ttf";
    if (FileUtils::exists(regularCjkPath)) {
        return regularCjkPath;
    }

    return defaultPath;
}

//********************
// Fonts::openAllFonts
//********************
void Fonts::openAllFonts(const std::string &_rootPath, SDL_Shared<SDL_Renderer> renderer) {
    fonts.clear();
    rootPath = _rootPath;
    medPath = getFontPathForCurrentLanguage(rootPath, FONT_MED);
    boldPath = getFontPathForCurrentLanguage(rootPath, FONT_BOLD);

    for (auto fontInfo : allFontInfos) {
        string path;
        if (fontInfo.fontType == FONT_MED)
            path = medPath;
        else
            path = boldPath;
        fonts[fontInfo.fontEnum] = openNewSharedCachedFont(path, fontInfo.size, renderer);
        fontInfos[fontInfo.fontEnum] = fontInfo;
    }
}

//********************
// SizesOfBoldThemeFont::AddFont
// If you ever need to change this to handle both bold and medium fonts change the map key to pair<FontType, pointSize>
// This class is used by ps_meta.cpp to make the game title font smaller if the game name is do long that it
// displays beyond the right edge of the screen.
//********************

//********************
// SizesOfBoldThemeFont::AddFont
//********************
FC_Font_Shared SizesOfBoldThemeFont::AddFont(int size, FC_Font_Shared font) {
    auto it = boldFonts.find(size);
    if (it != boldFonts.end())
        return it->second;
    else {
        boldFonts[size] = font; // add the passed font as a new font size font
        return font;
    }
}

//********************
// SizesOfBoldThemeFont::GetFont
//********************
FC_Font_Shared SizesOfBoldThemeFont::GetFont(int size, const Fonts &fonts) {
    auto it = boldFonts.find(size);
    if (it != boldFonts.end())
        return it->second; // we already have that size
    else {
        FC_Font_Shared font = fonts.openSpecificSharedCachedFont(FONT_BOLD, size);
        boldFonts[size] = font;
        return font;
    }
}
