#include "gui_font.h"
#include "../environment.h"
#include "../utils/file_utils.h"

using namespace std;

string Fonts::getThemeFontPath(const string &themePath, const string &fontName) {
    if (themePath == "" || fontName == "") {
        return "";
    }

    string fontPath = themePath + fontName;
    if (FileUtils::exists(fontPath)) {
        return fontPath;
    }

    string baseName = FileUtils::getFileNameFromPath(fontName);
    fontPath = themePath + "fonts" + sep + baseName;
    if (FileUtils::exists(fontPath)) {
        return fontPath;
    }

    fontPath = themePath + "font" + sep + baseName;
    if (FileUtils::exists(fontPath)) {
        return fontPath;
    }

    return "";
}

string Fonts::getRetroarchFontPath(const string &fontName) {
    if (fontName == "") {
        return "";
    }

    string baseName = FileUtils::getFileNameFromPath(fontName);
    string candidates[] = {
        Env::getPathToRetroarchDir() + sep + "fonts" + sep + baseName,
        Env::getPathToUSBRoot() + sep + ".retroarch" + sep + "fonts" + sep + baseName,
        Env::getPathToUSBRoot() + sep + ".retroarch-assets" + sep + "fonts" + sep + baseName,
    };

    for (const string &candidate : candidates) {
        if (FileUtils::exists(candidate)) {
            return candidate;
        }
    }

    return "";
}

string Fonts::getFirstAvailableFontPath(const string &path) {
    DirEntries files = FileUtils::diru_FilesOnly(path);
    for (const DirEntry &entry : files) {
        if (FileUtils::matchExtension(entry.name, "ttf") || FileUtils::matchExtension(entry.name, "otf")) {
            return path + sep + entry.name;
        }
    }

    return "";
}

string Fonts::getResourceFontPath(const string &rootPath, const string &fontName) {
    if (rootPath == "" || fontName == "") {
        return "";
    }

    string fontsDirPath = rootPath + sep + "fonts" + sep + fontName;
    if (FileUtils::exists(fontsDirPath)) {
        return fontsDirPath;
    }

    string legacyFontDirPath = rootPath + sep + "font" + sep + fontName;
    if (FileUtils::exists(legacyFontDirPath)) {
        return legacyFontDirPath;
    }

    string retroarchFontPath = getRetroarchFontPath(fontName);
    if (retroarchFontPath != "") {
        return retroarchFontPath;
    }

    return rootPath + sep + fontName;
}
