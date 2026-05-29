//
// Created by screemer on 2018-12-19.
//

#include "gui.h"
#include "gui_about.h"
#include "gui_splash.h"
#include "menus/gui_options_menu.h"
#include "menus/gui_mem_cards_menu.h"
#include "menus/gui_game_manager_menu.h"
#include "gui_confirm.h"
#include "gui_action_text.h"
#include "gui_text.h"
#include <SDL2/SDL_image.h>
#include "../launcher/gui_launcher.h"
#include "gui_pad_test.h"
#include <unistd.h>
#include <iostream>
#include "../log.h"
#include "../system/process_utils.h"
#include "../system/storage_info.h"
#include "../utils/string_utils.h"
#include <algorithm>
#include <iomanip>
#include <json.h>
#include "../nlohmann/fifo_map.h"

using namespace std;
using namespace nlohmann;

// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
template <class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
using ordered_json = basic_json<my_workaround_fifo_map>;

#define RA_PLAYLIST "AutoBleem.lpl"

namespace {
struct NextMusicPlaybackState {
    bool custom = false;
    int frequency = 32000;
    string path;
    bool enabled = false;
    int loops = 0;
    string resolvedPath;
};

NextMusicPlaybackState resolveNextMusicPlaybackState(const Gui &gui, const string &themePath) {
    NextMusicPlaybackState nextState;
    nextState.path = gui.themeData.get("music");
    if (gui.cfg.inifile.get("music") != "--") {
        nextState.custom = true;
        nextState.path = gui.cfg.inifile.get("music");
    }

    if (FileUtils::getFileExtension(nextState.path) == "ogg") {
        nextState.frequency = 44100;
    }

    nextState.enabled = gui.cfg.inifile.get("nomusic") != "true" && gui.themeData.get("loop") != "-1";
    nextState.loops = nextState.custom ? -1 : (gui.themeData.get("loop") == "1" ? -1 : 0);
    nextState.resolvedPath =
        nextState.custom ? Env::getWorkingPath() + sep + "music/" + nextState.path : themePath + nextState.path;
    return nextState;
}

bool audioMatchesFrequency(int frequency) {
    int currentAudioFrequency, currentAudioChannels;
    Uint16 currentAudioFormat;
    return Mix_QuerySpec(&currentAudioFrequency, &currentAudioFormat, &currentAudioChannels) > 0 &&
           currentAudioFrequency == frequency;
}

bool matchesCurrentMusicPlayback(const Gui &gui, const NextMusicPlaybackState &nextState) {
    bool matches = gui.musicState.enabled == nextState.enabled && gui.musicState.custom == nextState.custom &&
                   gui.musicState.frequency == nextState.frequency && gui.musicState.path == nextState.path &&
                   gui.musicState.loops == nextState.loops && gui.musicState.resolvedPath == nextState.resolvedPath;
    if (nextState.enabled) {
        return matches && gui.music != nullptr;
    }
    return matches && gui.music == nullptr;
}
} // namespace

//********************
// GuiBase::GuiBase
//********************
GuiBase::GuiBase() {
    window =
        SDL_CreateWindow("AutoBleem", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)

#else
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif

    TTF_Init();
    sonyFonts.openAllFonts(Env::getSonyFontPath(), renderer);
    themeFonts.openAllFonts(getCurrentThemeFontPath(), renderer);
}

//********************
// GuiBase::~GuiBase
//********************
GuiBase::~GuiBase() { SDL_Quit(); }

//*******************************
// GuiBase::getCurrentThemePath
//*******************************
string GuiBase::getCurrentThemePath() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    string path = Env::getPathToThemesDir() + sep + cfg.inifile.values["theme"];
    if (!FileUtils::exists(path)) {
        path = Env::getSonyPath();
    }
    return path;
#else
    string path = "/media/themes/" + cfg.inifile.values["theme"] + "";
    if (!FileUtils::exists(path)) {
        path = "/usr/sony/share/data";
    }
    return path;
#endif
}

//*******************************
// GuiBase::getCurrentThemeImagePath
//*******************************
string GuiBase::getCurrentThemeImagePath() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    string path = getCurrentThemePath() + sep + "images";
    if (!FileUtils::exists(path)) {
        path = Env::getSonyPath() + sep + "images";
    }
    return path;
#else
    string path = "/media/themes/" + cfg.inifile.values["theme"] + "/images";
    if (!FileUtils::exists(path)) {
        path = "/usr/sony/share/data/images";
    }
    return path;
#endif
}

//*******************************
// GuiBase::getCurrentThemeSoundPath
//*******************************
string GuiBase::getCurrentThemeSoundPath() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    string path = getCurrentThemePath() + sep + "sounds";
    if (!FileUtils::exists(path)) {
        path = Env::getSonyPath() + sep + "sounds";
    }
    PLOG_DEBUG << path;
    return path;
#else
    string path = "/media/themes/" + cfg.inifile.values["theme"] + "/sounds";
    if (!FileUtils::exists(path)) {
        path = "/usr/sony/share/data/sounds";
    }
    return path;
#endif
}

//*******************************
// GuiBase::getCurrentThemeFontPath
//*******************************
string GuiBase::getCurrentThemeFontPath() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    string path = getCurrentThemePath() + sep + "fonts";
    if (!FileUtils::exists(path)) {
        path = getCurrentThemePath() + sep + "font";
    }
    if (!FileUtils::exists(path)) {
        path = Env::getSonyPath() + sep + "font";
    }
    return path;
#else
    string path = "/media/themes/" + cfg.inifile.values["theme"] + "/fonts";
    if (!FileUtils::exists(path)) {
        path = "/media/themes/" + cfg.inifile.values["theme"] + "/font";
    }
    if (!FileUtils::exists(path)) {
        path = "/usr/sony/share/data/font";
    }
    return path;
#endif
}

//*******************************
// Gui::splash
//*******************************
void Gui::splash(const string &message) {
    shared_ptr<Gui> gui(Gui::getInstance());
    gui->drawText(message);
}

extern "C" {
//*******************************
// Gui::splash
//*******************************
void splash(char *message) {
    shared_ptr<Gui> gui(Gui::getInstance());
    gui->drawText(message);
}
}

//*******************************
// Gui::getR
//*******************************
Uint8 Gui::getR(const string &val) { return atoi(StringUtils::getToken(val, ',', 0).c_str()); }

//*******************************
// Gui::getG
//*******************************
Uint8 Gui::getG(const string &val) { return atoi(StringUtils::getToken(val, ',', 1).c_str()); }

//*******************************
// Gui::getB
//*******************************
Uint8 Gui::getB(const string &val) { return atoi(StringUtils::getToken(val, ',', 2).c_str()); }

void Gui::stopAudio() {
    int numtimesopened, frequency, channels;
    Uint16 format;
    numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
    for (int i = 0; i < numtimesopened; i++) {
        Mix_CloseAudio();
    }
    while (Mix_QuerySpec(&frequency, &format, &channels)) {
        Mix_CloseAudio();
    }
}

void Gui::restartAudio(int freq) {
    int numtimesopened, frequency, channels;
    Uint16 format;
    numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
    for (int i = 0; i < numtimesopened; i++) {
        Mix_CloseAudio();
    }
    while (Mix_QuerySpec(&frequency, &format, &channels))
        ;

    if (Mix_OpenAudio(freq, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Unable to open audio: %s\n", Mix_GetError());
    }

    const char *driver_name = SDL_GetCurrentAudioDriver();

    if (driver_name) {
        printf("Audio subsystem initialized; driver = %s.\n", driver_name);
    } else {
        printf("Audio subsystem not initialized.\n");
    }
}

void Gui::playMusic(bool customMusic, string musicPath) {
    if (cfg.inifile.values["nomusic"] != "true")
        if (themeData.values["loop"] != "-1") {
            if (!customMusic) {
                music = Mix_LoadMUS((themePath + musicPath).c_str());
                if (music == nullptr) {
                    printf("Unable to load Music file: %s\n", Mix_GetError());
                }
                if (Mix_PlayMusic(music, themeData.values["loop"] == "1" ? -1 : 0) == -1) {
                    printf("Unable to play music file: %s\n", Mix_GetError());
                }
            } else {
                music = Mix_LoadMUS((Env::getWorkingPath() + sep + "music/" + musicPath).c_str());
                if (music == nullptr) {
                    printf("Unable to load Music file: %s\n", Mix_GetError());
                }
                if (Mix_PlayMusic(music, -1) == -1) {
                    printf("Unable to play music file: %s\n", Mix_GetError());
                }
            }
        }
}

void Gui::freeMusic() {
    if (music != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = nullptr;
    }
    musicState = MusicPlaybackState();
}
//*******************************
// Gui::loadThemeTexture
//*******************************
SDL_Shared<SDL_Texture> Gui::loadThemeTexture(const string &themePath, const string &defaultPath,
                                              const string &texname) {
    SDL_Shared<SDL_Texture> tex = nullptr;
    if (FileUtils::exists(themePath + themeData.values[texname])) {
        tex = IMG_LoadTexture(renderer, (themePath + themeData.values[texname]).c_str());
    } else {
        tex = IMG_LoadTexture(renderer, (defaultPath + defaultData.values[texname]).c_str());
    }
    return tex;
}

//*******************************
// Gui::loadAssets
//*******************************
void Gui::loadAssets(bool reloadMusic) {
    // check theme exists - otherwise back to aergb

    string defaultPath = Env::getPathToThemesDir() + sep + "default" + sep;
    themePath = getCurrentThemePath() + sep;

    PLOG_DEBUG << "Loading UI theme:" << themePath;
    if (!FileUtils::exists(themePath + "theme.ini")) {
        themePath = defaultPath;
        cfg.inifile.values["theme"] = "default";
        cfg.save();
    }

    defaultData.load(defaultPath + "theme.ini");
    themeData.load(defaultPath + "theme.ini");
    themeData.OverwriteAndAppend(themePath + "theme.ini"); // adds to default/theme.ini values

    bool reloading = false;

    if (backgroundImg != nullptr) {
        Mix_FreeChunk(cursor);
        Mix_FreeChunk(cancel);
        Mix_FreeChunk(home_down);
        Mix_FreeChunk(home_up);
        reloading = true;
        backgroundImg = nullptr;
    }

    logoRect.x = atoi(themeData.values["lpositionx"].c_str());
    logoRect.y = atoi(themeData.values["lpositiony"].c_str());
    logoRect.w = atoi(themeData.values["lw"].c_str());
    logoRect.h = atoi(themeData.values["lh"].c_str());

    backgroundImg = loadThemeTexture(themePath, defaultPath, "background");
    logo = loadThemeTexture(themePath, defaultPath, "logo");
    if (cfg.inifile.values["jewel"] != "none") {
        if (cfg.inifile.values["jewel"] == "default") {
            cdJewel = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "evoimg/nofilter.png").c_str());
        } else {
            cdJewel = IMG_LoadTexture(
                renderer, (Env::getWorkingPath() + sep + "evoimg/frames/" + cfg.inifile.values["jewel"]).c_str());
        }
    } else {
        cdJewel = nullptr;
    }

    buttonTextureMap["O"] = loadThemeTexture(themePath, defaultPath, "circle");
    buttonTextureMap["X"] = loadThemeTexture(themePath, defaultPath, "cross");
    buttonTextureMap["T"] = loadThemeTexture(themePath, defaultPath, "triangle");
    buttonTextureMap["S"] = loadThemeTexture(themePath, defaultPath, "square");
    buttonTextureMap["Select"] = loadThemeTexture(themePath, defaultPath, "select");
    buttonTextureMap["Start"] = loadThemeTexture(themePath, defaultPath, "start");
    buttonTextureMap["L1"] = loadThemeTexture(themePath, defaultPath, "l1");
    buttonTextureMap["R1"] = loadThemeTexture(themePath, defaultPath, "r1");
    buttonTextureMap["L2"] = loadThemeTexture(themePath, defaultPath, "l2");
    buttonTextureMap["R2"] = loadThemeTexture(themePath, defaultPath, "r2");
    buttonTextureMap["Check"] = loadThemeTexture(themePath, defaultPath, "check");
    buttonTextureMap["Uncheck"] = loadThemeTexture(themePath, defaultPath, "uncheck");
    buttonTextureMap["Esc"] = loadThemeTexture(themePath, defaultPath, "esc");
    buttonTextureMap["Enter"] = loadThemeTexture(themePath, defaultPath, "enter");
    buttonTextureMap["Tab"] = loadThemeTexture(themePath, defaultPath, "tab");

    string fontPath = GuiText::resolveSelectedBodyFontPath(Gui::getInstance());
    int fontSize = 0;
    string fontSizeString = themeData.values["fsize"];
    if (fontSizeString != "")
        fontSize = atoi(fontSizeString.c_str());
    themeFont = Fonts::openNewSharedCachedFont(fontPath, fontSize, renderer);
    themeFonts.openAllFontsFromFontFile(fontPath, renderer);
    sizesOfThemeFont.Init();

    NextMusicPlaybackState nextMusicState = resolveNextMusicPlaybackState(*this, themePath);
    if (reloadMusic &&
        (!audioMatchesFrequency(nextMusicState.frequency) || !matchesCurrentMusicPlayback(*this, nextMusicState))) {
        freeMusic();
        this->restartAudio(nextMusicState.frequency);
        this->playMusic(nextMusicState.custom, nextMusicState.path);
    }
    musicState.custom = nextMusicState.custom;
    musicState.frequency = nextMusicState.frequency;
    musicState.path = nextMusicState.path;
    musicState.enabled = nextMusicState.enabled;
    musicState.loops = nextMusicState.loops;
    musicState.resolvedPath = nextMusicState.resolvedPath;
    cursor = Mix_LoadWAV((this->getCurrentThemeSoundPath() + sep + "cursor.wav").c_str());
    cancel = Mix_LoadWAV((this->getCurrentThemeSoundPath() + sep + "cancel.wav").c_str());
    home_up = Mix_LoadWAV((this->getCurrentThemeSoundPath() + sep + "home_up.wav").c_str());
    home_down = Mix_LoadWAV((this->getCurrentThemeSoundPath() + sep + "home_down.wav").c_str());
    resume = Mix_LoadWAV((this->getCurrentThemeSoundPath() + sep + "resume_new.wav").c_str());
}

//*******************************
// Gui::hideMouseCursor
//*******************************
void Gui::hideMouseCursor() {

#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
#else
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
}

//*******************************
// Gui::criticalException
//*******************************
void Gui::criticalException(const string &text) {
    drawText(text);
    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            mapper.handleHotPlug(&e);
            mapper.handlePowerBtn(&e);

            if (e.type == SDL_QUIT)
                return;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                return;

            if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                return;
            }
        }
    }
}

//*******************************
// Gui::display
//*******************************
void Gui::display(bool forceScan, const string &_pathToGamesDir, Database *db, bool resume) {
    this->db = db;
    this->pathToGamesDir = _pathToGamesDir;
    this->forceScan = forceScan;

    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    printf("We compiled against SDL version %d.%d.%d ...\n", compiled.major, compiled.minor, compiled.patch);
    printf("But we are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);

    Mix_Init(0);
    TTF_Init();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    loadAssets();

    if (!resume) {
        auto *splashScreen = new GuiSplash(renderer);
        splashScreen->show();
        delete splashScreen;
        hideMouseCursor();
    } else {
        resumingGui = true;
    }
}

//*******************************
// Gui::saveSelection
//*******************************
void Gui::saveSelection() {
    ofstream os;
    string path = cfg.inifile.values["cfg"];
    os.open(path);
    os << "#!/bin/sh" << endl << endl;
    os << "AB_SELECTION=" << menuOption << endl;

    os.flush();
    os.close();
}

bool otherMenuShift = false;
bool powerOffShift = false;

//*******************************
// Gui::menuSelection
//*******************************
void Gui::menuSelection() {
    shared_ptr<Scanner> scanner(Scanner::getInstance());

    if (!coverdb->isValid()) {
        criticalException(_("WARNING: NO COVER DB FOUND. PRESS ANY BUTTON"));
    }
    otherMenuShift = false;
    powerOffShift = false;
    string mainMenu = "|@Start| " + _("AutoBleem") + " |@X| " + _("Re/Scan") + " ";
    mainMenu += "|@S| " + _("RetroArch") + " ";
    mainMenu += "|@T| " + _("About") + " |@Select| " + _("Options") + " ";
    mainMenu += "|@L1| " + _("Advanced");
    mainMenu += " |@L2| + |@R2| " + _("Power Off");

    string forceScanMenu = _("Games changed. Press") + " |@X| " + _("to scan");
    string otherMenu;

    otherMenu += "|@X| " + _("Memory Cards") + " |@O| " + _("Game Manager");

    string gamepadNotice = "";
    if (SDL_NumJoysticks() > mapper.getActivePadNum()) {
        gamepadNotice = _("NOTICE: At least one connected gamepad is not recognized.");
    }

    if (!forceScan) {
        drawText(mainMenu, gamepadNotice);

    } else {
        drawText(forceScanMenu, gamepadNotice);
    }

    bool menuVisible = true;
    while (menuVisible) {
        if (startingGame) {
            drawText(runningGame->title);
            this->menuOption = MENU_OPTION_START;
            menuVisible = false;
            startingGame = false;
            return;
        }

        if (resumingGui) {
            auto launcherScreen = new GuiLauncher(renderer);
            launcherScreen->show();
            delete launcherScreen;
            drawText("");
            resumingGui = false;
            menuSelection();
            menuVisible = false;
        }
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            mapper.handleHotPlug(&e);
            mapper.handlePowerBtn(&e);
            // this is for pc Only
            if (e.type == SDL_QUIT) {
                menuVisible = false;
            }
            switch (e.type) {

            case SDL_CONTROLLERBUTTONUP:
                if (!forceScan) {
                    if (e.cbutton.button == SDL_BTN_L1) {
                        Mix_PlayChannel(-1, cursor, 0);
                        drawText(mainMenu);
                        otherMenuShift = false;
                    }
                    if (e.cbutton.button == SDL_BTN_L2) {
                        Mix_PlayChannel(-1, cursor, 0);
                        powerOffShift = false;
                    }
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                if (!forceScan) {
                    if (e.cbutton.button == SDL_BTN_L1) {
                        Mix_PlayChannel(-1, cursor, 0);
                        drawText(otherMenu);
                        otherMenuShift = true;
                    }
                    if (e.cbutton.button == SDL_BTN_L2) {
                        Mix_PlayChannel(-1, cursor, 0);
                        powerOffShift = true;
                    }
                }

                if (powerOffShift) {
                    if (e.cbutton.button == SDL_BTN_R2) {
                        Mix_PlayChannel(-1, cursor, 0);
                        drawText(_("POWERING OFF... PLEASE WAIT"));
                        System::shutdown();
                    };
                }

                if (!otherMenuShift) {
                    if (!forceScan)
                        if (e.cbutton.button == SDL_BTN_START) {
                            if (lastSet < 0) {
                                lastSet = SET_PS1;
                                lastSelIndex = 0;
                                resumingGui = false;
                            }
                            Mix_PlayChannel(-1, cursor, 0);
                            drawText(_("Starting EvolutionUI"));
                            loadAssets(false);
                            auto launcherScreen = new GuiLauncher(renderer);
                            launcherScreen->show();
                            delete launcherScreen;

                            menuSelection();
                            menuVisible = false;
                        };

                    if (!forceScan)
                        if (e.cbutton.button == SDL_BTN_SQUARE) {
                            Mix_PlayChannel(-1, cursor, 0);
                            if (!FileUtils::exists(Env::getPathToRetroarchDir() + sep + "retroarch")) {

                                auto confirm = new GuiConfirm(renderer);
                                confirm->label = _("RetroArch is not installed");
                                confirm->show();
                                bool result = confirm->result;
                                delete confirm;
                                if (result) {
                                    this->menuOption = MENU_OPTION_RETRO;
                                    menuVisible = false;
                                } else {
                                    menuSelection();
                                    menuVisible = false;
                                }
                            } else {
                                exportDBToRetroarch();
                                this->menuOption = MENU_OPTION_RETRO;
                                menuVisible = false;
                            }
                        };

                    if (e.cbutton.button == SDL_BTN_CROSS) {
                        Mix_PlayChannel(-1, cursor, 0);
                        this->menuOption = MENU_OPTION_SCAN;

                        menuVisible = false;
                    };
                    if (e.cbutton.button == SDL_BTN_TRIANGLE) {
                        Mix_PlayChannel(-1, cursor, 0);
                        auto *aboutScreen = new GuiAbout(renderer);
                        aboutScreen->show();
                        delete aboutScreen;

                        menuSelection();
                        menuVisible = false;
                    };
                    if (e.cbutton.button == SDL_BTN_SELECT) {
                        Mix_PlayChannel(-1, cursor, 0);
                        auto options = new GuiOptions(renderer);
                        options->show();
                        delete options;
                        menuSelection();
                        menuVisible = false;
                    };
                    break;
                } else {
                    if (e.cbutton.button == SDL_BTN_CROSS) {
                        Mix_PlayChannel(-1, cursor, 0);
                        auto memcardsScreen = new GuiMemcards(renderer);
                        memcardsScreen->show();
                        delete memcardsScreen;

                        menuSelection();
                        menuVisible = false;
                    };

                    if (e.cbutton.button == SDL_BTN_CIRCLE) {
                        Mix_PlayChannel(-1, cursor, 0);
                        auto managerScreen = new GuiManager(renderer);
                        managerScreen->show();
                        delete managerScreen;

                        menuSelection();
                        menuVisible = false;
                    };
                }
            }
        }
    }
}

//*******************************
// Gui::finish
//*******************************
void Gui::finish() {

    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(300);
        while (Mix_PlayingMusic()) {
        }
    } else {
        usleep(300 * TicksPerSecond);
    }

    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_FreeChunk(cursor);
    Mix_FreeChunk(cancel);
    Mix_FreeChunk(home_down);
    Mix_FreeChunk(home_up);
    Mix_CloseAudio();
    music = nullptr;
    backgroundImg = nullptr;
}

//*******************************
// Gui::exportDBToRetroarch
//*******************************
void Gui::exportDBToRetroarch() {
    ordered_json j;
    j["version"] = "1.0";

    PsGames gamesList;
    db->getGames(&gamesList);
    sort(gamesList.begin(), gamesList.end(), sortByTitle);

    ordered_json items = ordered_json::array();
    // copy the gamesList into json object
    for_each(begin(gamesList), end(gamesList), [&](PsGamePtr &game) {
        ordered_json item = ordered_json::object();

        string gameFile = (game->folder + sep + game->base);
        if (!FileUtils::matchExtension(game->base, ".pbp") && !FileUtils::matchExtension(game->base, ".chd")) {
            gameFile += ".cue";
        }
        gameFile += "";

        string base;
        if (FileUtils::isPBPFile(game->base)) {
            base = game->base.substr(0, game->base.length() - 4);
        } else {
            base = game->base;
        }
        if (FileUtils::exists(game->folder + sep + base + ".m3u")) {
            gameFile = game->folder + sep + base + ".m3u";
        }

        item["path"] = gameFile;
        item["label"] = game->title;
        item["core_path"] = Env::getPathToRetroarchCoreFile();
        item["core_name"] = "DETECT";
        item["crc32"] = "00000000|crc";
        item["db_name"] = RA_PLAYLIST;

        items.push_back(item);
    });

    j["items"] = items;

    PLOG_DEBUG << j.dump();
    std::ofstream o(Env::getPathToRetroarchDir() + sep + "playlists/" + RA_PLAYLIST);
    o << std::setw(2) << j << std::endl;
    o.flush();
    o.close();
}

//*******************************
// Gui::getOpscreenRectOfTheme
//*******************************
SDL_Rect Gui::getOpscreenRectOfTheme() {
    SDL_Rect rect;
    rect.x = atoi(themeData.values["opscreenx"].c_str());
    rect.y = atoi(themeData.values["opscreeny"].c_str());
    rect.w = atoi(themeData.values["opscreenw"].c_str());
    rect.h = atoi(themeData.values["opscreenh"].c_str());

    return rect;
}

int Gui::getContentTopY(int padding) { return getOpscreenRectOfTheme().y + padding; }

int Gui::getContentBottomY(int padding) {
    SDL_Rect rect = getOpscreenRectOfTheme();
    return rect.y + rect.h - padding;
}

//*******************************
// Gui::getTextRectOfTheme
//*******************************
SDL_Rect Gui::getTextRectOfTheme() {
    SDL_Rect rect;
    rect.x = atoi(themeData.values["textx"].c_str());
    rect.y = atoi(themeData.values["texty"].c_str());
    rect.w = atoi(themeData.values["textw"].c_str());
    rect.h = atoi(themeData.values["texth"].c_str());

    return rect;
}

//*******************************
// Gui::getCheckIconWidth
// returns the width of the check/uncheck icon textures
//*******************************
int Gui::getCheckIconWidth() {
    int checkIconWidth = 0;
    int checkIconHeight = 0;
    auto it = buttonTextureMap.find("Check");
    if (it != buttonTextureMap.end()) {
        SDL_QueryTexture(it->second, nullptr, nullptr, &checkIconWidth, &checkIconHeight);
    } else {
        PLOG_DEBUG << "missing check icon";
        assert(false);
    }

    return checkIconWidth;
}

//*******************************
// Gui::align_xPosition
//*******************************
int Gui::align_xPosition(XAlignment xAlign, int x, int width) {
    if (xAlign == XALIGN_CENTER) {
        x = (SCREEN_WIDTH / 2) - width / 2;
    } else if (xAlign == XALIGN_RIGHT) {
        x = SCREEN_WIDTH - x - width;
    }

    return x;
}

//*******************************
// Gui::renderTextLineOptions
//*******************************
int Gui::renderTextLineOptions(const string &_text, int line, int yoffset, XAlignment xAlign, int xoffset) {
    string text = _text;

    // if there is a check or uncheck icon, flag which one and remove the emoji toekn from the string
    int button = -1;
    if (text.find("|@Check|") != std::string::npos) {
        button = 1;
    }
    if (text.find("|@Uncheck|") != std::string::npos) {
        button = 0;
    }
    if (button != -1) {
        text = text.substr(0, text.find("|"));
    }

    SDL_Rect opscreen = getOpscreenRectOfTheme();
    int fontSize = getThemeFontSize();

    int textWidth = opscreen.w - xoffset - 20;
    if (button != -1) {
        textWidth -= getCheckIconWidth() + 20;
    }

    if (button == -1) {
        // render the text string without the check/uncheck icon
        int h = renderFittedTextLine(text, line, yoffset, xAlign, xoffset, textWidth, fontSize, 12, themeFont);
        return h; // there is no check/uncheck emoji on this line
    }

    // render the check/uncheck icon on the right side of opscreen
    Uint16 fontHeight = FC_GetLineHeight(themeFont);

    int x = opscreen.x + opscreen.w - 10 - getCheckIconWidth();
    int y = (fontHeight * line) + yoffset;
    if (line < 0) {
        y = -line;
    }

    FC_Font_Shared fittedFont = getFittingThemeFont(themeFont, fontSize, 12, text, textWidth);
    AllTextOrEmojiTokenInfo textInfo(fittedFont, text);
    AllTextOrEmojiTokenInfo buttonInfo(themeFont, button == 1 ? "|@Check|" : "|@Uncheck|");
    int rowHeight = std::max(textInfo.totalSize.h, buttonInfo.totalSize.h);
    int textX = opscreen.x + 10 + xoffset;
    textInfo.render(textX, y + ((rowHeight - textInfo.totalSize.h) / 2) + getTextVisualYOffset(fittedFont), xAlign);

    if (button == 1) {
        buttonInfo.render(x, y + ((rowHeight - buttonInfo.totalSize.h) / 2));
    } else if (button == 0) {
        buttonInfo.render(x, y + ((rowHeight - buttonInfo.totalSize.h) / 2));
    }

    return rowHeight;
}

//*******************************
// Gui::renderSelectionBox
//*******************************
void Gui::renderSelectionBox(int line, int yoffset, int xoffset, FC_Font_Shared font) {
    if (!font)
        font = themeFont;

    Uint16 fontHeight = FC_GetLineHeight(font);
    int selectionOffset = getSelectionBoxYOffset(font);
    int y = yoffset + fontHeight * line;
    if (line < 0) {
        y = -line;
    }
    renderSelectionBoxAtY(y - selectionOffset, fontHeight + (selectionOffset * 2), xoffset, font);
}

void Gui::renderSelectionBoxAtY(int y, int height, int xoffset, FC_Font_Shared font) {
    if (!font)
        font = themeFont;

    string fg = themeData.values["text_fg"];
    SDL_Rect opscreen = getOpscreenRectOfTheme();
    SDL_Rect rectSelection;
    rectSelection.x = opscreen.x + 5 + xoffset;
    rectSelection.y = y;
    rectSelection.w = opscreen.w - 10 - xoffset;
    rectSelection.h = height;

    SDL_SetRenderDrawColor(renderer, getR(fg), getG(fg), getB(fg), 255);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawRect(renderer, &rectSelection);
}

//*******************************
// Gui::renderLabelBox
//*******************************
void Gui::renderLabelBox(int line, int yoffset) {
    string bg = themeData.values["label_bg"];
    Uint16 fontHeight = FC_GetLineHeight(themeFont);
    SDL_Rect opscreen = getOpscreenRectOfTheme();
    SDL_Rect rectSelection;
    rectSelection.x = opscreen.x + 5;
    rectSelection.y = yoffset + fontHeight * (line);
    rectSelection.w = opscreen.w - 10;
    rectSelection.h = fontHeight;

    SDL_SetRenderDrawColor(renderer, getR(bg), getG(bg), getB(bg), atoi(themeData.values["keyalpha"].c_str()));
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &rectSelection);
}

//*******************************
// Gui::renderTextChar
//*******************************
void Gui::renderTextChar(const string &text, int line, int yoffset, int x) {
    Uint16 fontHeight = FC_GetLineHeight(themeFont);
    int y = (fontHeight * line) + yoffset;
    FC_DrawAlign(themeFont, renderer, x, y, FC_ALIGN_LEFT, "%s", text.c_str());
}

//*******************************
// Gui::renderFreeSpace
//*******************************
void Gui::renderFreeSpace(int y) {
    if (y < 0) {
        y = atoi(themeData.values["fsposy"].c_str());
    }
    if (y <= 0) {
        y = 35;
    }
    renderFittedText_WithColor(themeFonts[FONT_20_BOLD], _("Free space") + " : " + System::getStorageInfo().formatted(),
                               0, y, SCREEN_WIDTH - 80, 20, 12, getTitleTextColor(), XALIGN_CENTER);
}

//*******************************
// Gui::renderBackground
//*******************************
void Gui::renderBackground() {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, backgroundImg, nullptr, &backgroundRect);
}

//*******************************
// Gui::renderLogo
//*******************************
int Gui::renderLogo(bool small) {
    if (!small) {
        SDL_RenderCopy(renderer, logo, nullptr, &logoRect);
        return 0;
    } else {
        SDL_Rect rect;
        rect.x = atoi(themeData.values["opscreenx"].c_str());
        rect.y = atoi(themeData.values["opscreeny"].c_str());
        rect.w = logoRect.w / 3;
        rect.h = logoRect.h / 3;
        SDL_RenderCopy(renderer, logo, nullptr, &rect);
        return rect.y + rect.h;
    }
}

//*******************************
// Gui::renderActionText
//*******************************
int Gui::renderActionText(FC_Font_Shared baseFont, const string &text, int y, int maxWidth, int maxSize, int minSize) {
    GuiActionText::Layout layout =
        GuiActionText::getLayout(Gui::getInstance(), baseFont, text, maxWidth, maxSize, minSize);
    int x = (SCREEN_WIDTH / 2) - (layout.width / 2);
    GuiActionText::renderLayout(layout, x, y);

    return layout.height;
}

//*******************************
// Gui::renderStatus
//*******************************
void Gui::renderStatus(const string &text, int posy) {
    string bg = themeData.values["main_bg"];
    SDL_Rect themeRect = getTextRectOfTheme();

    int y = atoi(themeData.values["ttop"].c_str());
    if (posy != -1)
        y = posy; // override the bottom status y position.  so far this has never been used.

    GuiActionText::Layout layout =
        GuiActionText::getLayout(Gui::getInstance(), themeFonts[FONT_20_BOLD], text, themeRect.w - 20, 20, 12);
    int verticalPadding = 6;
    SDL_Rect rect = themeRect;
    rect.h = std::max(themeRect.h, layout.height + (verticalPadding * 2));
    rect.y = y - verticalPadding;
    if (rect.y + rect.h > SCREEN_HEIGHT) {
        rect.y = SCREEN_HEIGHT - rect.h;
    }
    if (rect.y < 0) {
        rect.y = 0;
    }

    bool isBottomStatus = posy == -1 || rect.y >= SCREEN_HEIGHT - 120;
    string alpha = isBottomStatus ? themeData.get("statusalpha", "70") : themeData.get("mainalpha", "170");
    int bgAlpha = atoi(alpha.c_str());
    if (bgAlpha > 0) {
        SDL_SetRenderDrawColor(renderer, getR(bg), getG(bg), getB(bg), bgAlpha);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &rect);
    }

    int textY = rect.y + ((rect.h - layout.height) / 2);
    int x = (SCREEN_WIDTH / 2) - (layout.width / 2);
    GuiActionText::renderLayout(layout, x, textY);
}

//*******************************
// Gui::renderTextBar
//*******************************
void Gui::renderTextBar() {
    string bg = themeData.values["main_bg"];
    SDL_SetRenderDrawColor(renderer, getR(bg), getG(bg), getB(bg), atoi(themeData.values["mainalpha"].c_str()));
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect rect2 = getOpscreenRectOfTheme();

    SDL_RenderFillRect(renderer, &rect2);
}

//*******************************
// Gui::drawText
//*******************************
void Gui::drawText(const string &text, const string &topLine) {
    renderBackground();
    renderLogo(false);
    renderStatus(text);
    renderStatus(topLine, 5);
    SDL_RenderPresent(renderer);
}
