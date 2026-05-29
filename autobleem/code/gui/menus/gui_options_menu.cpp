#include "gui_options_menu.h"
#include "../../utils/random_utils.h"
#include "../../environment.h"

using namespace std;

static bool hasChoice(const vector<string> &list, const string &choice) {
    for (const string &entry : list) {
        if (entry == choice) {
            return true;
        }
    }

    return false;
}

static void addFontChoicesFromDir(vector<string> &list, const string &path) {
    DirEntries files = FileUtils::diru_FilesOnly(path);
    for (const DirEntry &entry : files) {
        if (!FileUtils::matchExtension(entry.name, "ttf") && !FileUtils::matchExtension(entry.name, "otf")) {
            continue;
        }

        if (!hasChoice(list, entry.name)) {
            list.push_back(entry.name);
        }
    }
}

static string getWrappedChoice(OptionsInfo &info, const string &current, bool next) {
    const vector<string> &list = info.choices;
    if (list.empty()) {
        return "";
    }

    int pos = 0;
    for (int i = 0; i < list.size(); i++) {
        if (list[i] == current) {
            pos = i;
            break;
        }
    }

    if (next) {
        pos++;
        if (pos >= list.size()) {
            pos = 0;
        }
    } else {
        pos--;
        if (pos < 0) {
            pos = list.size() - 1;
        }
    }

    return list[pos];
}

static bool shouldReloadMusicForOption(int id) {
    return id == CFG_THEME || id == CFG_MUSIC || id == CFG_ENABLE_BACKGROUND_MUSIC;
}

void GuiOptions::applyOptionSideEffects(int id, const string &nextValue) {
    if (id == CFG_THEME || id == CFG_THEME_FONT || id == CFG_FONT) {
        gui->loadAssets(shouldReloadMusicForOption(id));
        font = gui->themeFont;
    } else if (id == CFG_LANG) {
        lang->load(nextValue);
        gui->loadAssets(false);
        font = gui->themeFont;
    } else if (id == CFG_MUSIC || id == CFG_ENABLE_BACKGROUND_MUSIC) {
        gui->loadAssets();
    }
}

string GuiOptions::getStatusLine() {
    auto id = lines[selected].id;
    if (id == CFG_THEME || id == CFG_FONT || id == CFG_MUSIC)
        return "|@X| " + _("OK") + "     " + "|@O| " + _("Cancel") + "  " + "|@Start|   " + _("Random") + "|";
    else
        return "|@X| " + _("OK") + "     " + "|@O| " + _("Cancel") + "|";
}

//*******************************
// GuiOptions::getThemes
//*******************************
vector<string> GuiOptions::getThemes() {
    vector<string> list;
    string uiThemePath = Env::getPathToThemesDir();
    DirEntries uiThemeFolders = FileUtils::diru_DirsOnly(uiThemePath);
    for (const DirEntry &entry : uiThemeFolders) {
        if (FileUtils::exists(uiThemePath + sep + entry.name + sep + "theme.ini")) {
            list.push_back(entry.name); // add the theme dir name
        }
    }

    return list;
}

//*******************************
// GuiOptions::getFonts
//*******************************
vector<string> GuiOptions::getFonts() {
    vector<string> list;
    list.push_back("--");

    addFontChoicesFromDir(list, Env::getPathToRetroarchDir() + sep + "fonts");
    addFontChoicesFromDir(list, Env::getWorkingPath() + sep + "fonts");
    addFontChoicesFromDir(list, gui->getCurrentThemeFontPath());

    return list;
}

//*******************************
// GuiOptions::getJewels
//*******************************
vector<string> GuiOptions::getJewels() {
    vector<string> list;
    DirEntries folders = FileUtils::diru_FilesOnly(Env::getWorkingPath() + sep + "evoimg/frames");
    for (const DirEntry &entry : folders) {
        if (FileUtils::getFileExtension(entry.name) == "png") {
            list.push_back(entry.name);
        }
    }

    return list;
}

//*******************************
// GuiOptions::getMusic
//*******************************
vector<string> GuiOptions::getMusic() {
    vector<string> list;
    list.push_back("--");
    DirEntries folders = FileUtils::diru_FilesOnly(Env::getWorkingPath() + sep + "music");
    for (const DirEntry &entry : folders) {
        if (FileUtils::getFileExtension(entry.name) == "ogg") {
            list.push_back(entry.name);
        }
    }

    return list;
}

//*******************************
// GuiOptions::getTimeoutValues
//*******************************
vector<string> GuiOptions::getTimeoutValues() {
    vector<string> list;
    for (int i = 0; i <= 20; ++i) {
        list.push_back(to_string(i));
    }

    return list;
}

//*******************************
// GuiOptions::fill
//*******************************
void GuiOptions::fill() {
    // this is filled once and not on every render.
    // save the current lang and switch to English.  we need the "Prefix:" to be scanned in English for English.txt
    // getLineText() will do the translation
    string saveCurrentLang = lang->currentLang;
    lang->load(DEFAULT_LANG);

    lines.emplace_back(CFG_THEME, _("AutoBleem Theme"), "theme", false, getThemes());
    lines.emplace_back(CFG_THEME_FONT, _("Use Font from Theme"), "themefont", true, vector<string>({"false", "true"}));
    lines.emplace_back(CFG_FONT, _("Font"), "font", false, getFonts());
    lines.emplace_back(CFG_SHOW_ORIGAMES, _("Show Internal Games"), "origames", true,
                       vector<string>({"false", "true"}));
    lines.emplace_back(CFG_JEWEL, _("Cover Style"), "jewel", false, getJewels());
    lines.emplace_back(CFG_MUSIC, _("Music"), "music", false, getMusic());
    lines.emplace_back(CFG_ENABLE_BACKGROUND_MUSIC, _("Background Music"), "nomusic", true,
                       vector<string>({"true", "false"}));
    lines.emplace_back(CFG_WIDESCREEN, _("Widescreen"), "aspect", true, vector<string>({"false", "true"}));
    lines.emplace_back(CFG_GFX_FILTER, _("GFX Filter"), "mip", true, vector<string>({"true", "false"}));
    lines.emplace_back(CFG_RACONFIG, _("Update RA Config"), "raconfig", true, vector<string>({"false", "true"}));
    lines.emplace_back(CFG_PLAY_ALL_PSX_WITH_RA, _("Play all PSX games with RA"), "play_all_psx_with_ra", true,
                       vector<string>({"false", "true"}));
    lines.emplace_back(CFG_SHOWINGTIMEOUT, _("Showing Timeout (0 for no timeout)"), "showingtimeout", false,
                       getTimeoutValues());
    lines.emplace_back(CFG_LANG, _("Language"), "language", false, lang->getListOfLanguages());

    lang->load(saveCurrentLang);
}

//*******************************
// GuiOptions::init
//*******************************
void GuiOptions::init() {
    GuiOptionsMenuBase::init(); // call the base class init()
    firstRow = 1;
    lines.clear();
    fill();
}

void GuiOptions::render() {
    SDL_RenderClear(renderer);
    gui->renderBackground();
    gui->renderTextBar();

    if (firstRender) {
        computePagePosition();
        firstRender = false;
    }

    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    int visibleCount = 0;
    if (getVerticalSize() > 0) {
        visibleCount = lastVisibleIndex - firstVisibleIndex + 1;
        if (visibleCount > getVerticalSize()) {
            visibleCount = getVerticalSize();
        }
    }

    int fontHeight = FC_GetLineHeight(font);
    int lineCount = visibleCount + 1;
    int firstLineY = gui->getContentTopY();
    int lastLineY = gui->getContentBottomY() - fontHeight;
    int rowSpacing = fontHeight;
    if (lineCount > 1 && lastLineY > firstLineY) {
        rowSpacing = (lastLineY - firstLineY) / (lineCount - 1);
        if (rowSpacing < fontHeight) {
            rowSpacing = fontHeight;
        }
    }

    gui->renderTitleLine(getTitle(), -firstLineY, 0);

    int row = 1;
    for (int i = firstVisibleIndex; i <= lastVisibleIndex; i++) {
        if (i < 0 || i >= getVerticalSize()) {
            break;
        }
        string line = getLineText(lines[i]);
        gui->renderTextLineOptions(line, -(firstLineY + (rowSpacing * row)), 0, XALIGN_LEFT);
        row++;
    }

    if (getVerticalSize() > 0) {
        int selectedRow = selected - firstVisibleIndex + 1;
        int selectedY = firstLineY + (rowSpacing * selectedRow);
        int selectionHeight = std::max(fontHeight + (Gui::getSelectionBoxYOffset(font) * 2), rowSpacing - 2);
        int selectionY = selectedY - ((selectionHeight - fontHeight) / 2) - Gui::getSelectionBoxYOffset(font);
        gui->renderSelectionBoxAtY(selectionY, selectionHeight, 0, font);
    }

    gui->renderStatus(getStatusLine());
    SDL_RenderPresent(renderer);
}

//*******************************
// void GuiOptions::getLineText
//*******************************
std::string GuiOptions::getLineText(const OptionsInfo &info) {
    std::string temp = lang->translate(info.descriptionToTranslate) + ": ";
    auto value = gui->cfg.inifile.values[info.iniKey];
    if (info.keyIsBoolean) {
        temp += getBooleanSymbolText(info, value);
    } else if (info.id == CFG_FONT && (value == "" || value == "--")) {
        temp += _("Theme Default");
    } else {
        temp += value; // append the current text value in the options list
    }

    return temp;
}

//*******************************
// GuiOptions::doPrevNextOption
//*******************************
string GuiOptions::doPrevNextOption(OptionsInfo &info, bool next) {
    int id = info.id;

    string nextValue;
    if (id == CFG_FONT) {
        nextValue = getWrappedChoice(info, gui->cfg.inifile.values[info.iniKey], next);
        gui->cfg.inifile.values[info.iniKey] = nextValue;
    } else {
        nextValue = GuiOptionsMenuBase::doPrevNextOption(info, next);
    }

    applyOptionSideEffects(id, nextValue);

    return nextValue;
}

//*******************************
// string GuiOptions::doRandomOption()
// only a few lines will use this.  most will just return.
//*******************************
string GuiOptions::doRandomOption() {
    int id = lines[selected].id;
    if (id == CFG_THEME || id == CFG_FONT || id == CFG_MUSIC) {
        auto &choices = lines[selected].choices;
        unsigned int size = choices.size();
        if (size > 1)
            return doOptionIndex(RandomUtils::RandomGenerator::getInstance().generateInt(0, size - 1));
    }
    return "";
}

//*******************************
// string GuiOptions::doOptionIndex()
//*******************************
string GuiOptions::doOptionIndex(uint index) {
    if (validSelectedIndex()) {
        int id = lines[selected].id;
        // do the default action
        string nextValue = GuiOptionsMenuBase::doOptionIndex(index);

        applyOptionSideEffects(id, nextValue);

        return nextValue;
    } else
        return "";
}

//*******************************
// GuiOptions::doCircle_Pressed
//*******************************
void GuiOptions::doCircle_Pressed() {
    Mix_PlayChannel(-1, gui->cancel, 0);
    string cfg_path = Env::getWorkingPath() + sep + "config.ini";
    gui->cfg.inifile.load(cfg_path);                 // restore the original config.ini settings
    lang->load(gui->cfg.inifile.values["language"]); // restore the original lang
    gui->loadAssets();                               // restore original themes
    menuVisible = false;
    exitCode = -1;
}

//*******************************
// GuiOptions::doCross_Pressed
//*******************************
void GuiOptions::doCross_Pressed() {
    Mix_PlayChannel(-1, gui->cancel, 0);
    gui->cfg.save();
    menuVisible = false;
    exitCode = 0;
}

//*******************************
// GuiOptions::doJoyRight
//*******************************
void GuiOptions::doJoyRight() {
    do {
        doKeyRight();
        render();
    } while (fastForwardUntilAnotherEvent());
}

//*******************************
// GuiOptions::doJoyLeft
//*******************************
void GuiOptions::doJoyLeft() {
    do {
        doKeyLeft();
        render();
    } while (fastForwardUntilAnotherEvent());
}

//*******************************
// GuiOptions::doKeyRight
//*******************************
void GuiOptions::doKeyRight() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    doPrevNextOption(true);
}

//*******************************
// GuiOptions::doKeyLeft
//*******************************
void GuiOptions::doKeyLeft() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    doPrevNextOption(false);
}
