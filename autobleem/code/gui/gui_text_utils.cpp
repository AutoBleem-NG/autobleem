#include "gui_text.h"
#include "../environment.h"
#include "../log.h"
#include "../utils/file_utils.h"
#include <vector>

using namespace std;

namespace GuiText {
string joinLines(const vector<string> &lines, const string &separator) {
    string text;
    for (const string &line : lines) {
        if (line == "") {
            continue;
        }
        if (text != "") {
            text += separator;
        }
        text += line;
    }

    return text;
}

static string resolveNamedFontPath(const string &themePath, const string &fontName) {
    if (fontName == "") {
        return "";
    }

    string fontPath = Fonts::getThemeFontPath(themePath, fontName);
    if (fontPath != "") {
        return fontPath;
    }

    fontPath = Fonts::getResourceFontPath(Env::getWorkingPath(), fontName);
    if (FileUtils::exists(fontPath)) {
        return fontPath;
    }

    return "";
}

static string resolveFirstAvailableFontPath(const string &themePath) {
    const vector<string> fontDirectories = {
        Env::getPathToRetroarchDir() + sep + "fonts",
        Env::getWorkingPath() + sep + "fonts",
        themePath + "fonts",
        themePath + "font",
    };

    for (const string &fontDirectory : fontDirectories) {
        string fontPath = Fonts::getFirstAvailableFontPath(fontDirectory);
        if (fontPath != "") {
            return fontPath;
        }
    }

    return "";
}

string resolveThemeFontPath(const string &themePath, const string &fontName) {
    string fontPath = resolveNamedFontPath(themePath, fontName);
    if (fontPath != "") {
        return fontPath;
    }

    fontPath = resolveFirstAvailableFontPath(themePath);
    if (fontPath != "") {
        PLOG_WARNING << "Font not found: " << fontName << "; using fallback font: " << fontPath;
        return fontPath;
    }

    PLOG_ERROR << "Font not found and no fallback font is available: " << fontName;
    return fontPath;
}

string resolveFontWithFallback(const string &fontName, const string &fallbackFontName, const string &themePath) {
    string fontPath = resolveNamedFontPath(themePath, fontName);
    if (fontPath != "") {
        return fontPath;
    }

    fontPath = resolveNamedFontPath(themePath, fallbackFontName);
    if (fontPath != "") {
        PLOG_WARNING << "Font not found: " << fontName << "; using fallback font: " << fontPath;
        return fontPath;
    }

    fontPath = resolveFirstAvailableFontPath(themePath);
    if (fontPath != "") {
        PLOG_WARNING << "Fonts not found: " << fontName << ", " << fallbackFontName
                     << "; using fallback font: " << fontPath;
        return fontPath;
    }

    PLOG_ERROR << "Fonts not found and no fallback font is available: " << fontName << ", " << fallbackFontName;
    return Fonts::getResourceFontPath(Env::getWorkingPath(), fallbackFontName);
}
} // namespace GuiText
