//
// Created by screemer on 2019-01-25.
//

#include "gui_game_editor_menu.h"
#include "../gui.h"
#include "../gui_layout.h"
#include "../gui_keyboard.h"
#include "../gui_select_mem_card.h"
#include "../../engine/cfg_processor.h"
#include "../../engine/mem_card.h"
#include "../../launcher/thumbnail_lookup.h"
#include <SDL2/SDL_image.h>
#include "../../lang.h"
#include <sstream>
#include "../../environment.h"
#include "../../lightgun_games.h"
#include "../../system/process_utils.h"
#include <algorithm>
#include <vector>

using namespace std;

#define OPT_FIRST 5
#define OPT_FAVORITE 5
#define OPT_LIGHTGUN 6
#define OPT_PLAY_USING_RA 7
#define OPT_LOCK 8
#define OPT_HIGHRES 9
#define OPT_SPEEDHACK 10
#define OPT_SCANLINES 11
#define OPT_SCANLINELV 12
#define OPT_CLOCK_PSX 13
#define OPT_FRAMESKIP 14
#define OPT_PLUGIN 15
#define OPT_INTERPOLATION 16
#define OPT_LAST 16

namespace {
const int editorPanelGap = 26;
const int editorPanelPadding = 16;
const int editorInfoPanelWidth = 390;
const int editorInfoLabelWidth = 112;
const int editorCoverSize = 226;
const int editorTitleMaxFontSize = 22;
const int editorTitleMinFontSize = 13;
const int editorInfoMaxFontSize = 16;
const int editorInfoMinFontSize = 10;
const int editorSettingsTitleMaxFontSize = 20;
const int editorSettingsTitleMinFontSize = 13;
const int editorSettingsMaxFontSize = 19;
const int editorSettingsMinFontSize = 12;
const int editorSettingsValueWidth = 170;
const int editorSettingsMinRowStep = 28;
const int editorSettingsRowGap = 6;
const SDL_Color editorPanelFillColor = {0, 0, 0, 118};
const SDL_Color editorPanelBorderColor = {255, 255, 255, 42};
const SDL_Color editorMutedTextColor = {205, 205, 205, 255};

// Cover-art resolution for the editor menu: a PNG dropped in the game's
// own folder wins (user override), then libretro-thumbnails Named_Boxarts
// (resolved through serial + title), then the bundled default cover.
SDL_Shared<SDL_Texture> loadCoverTexture(SDL_Shared<SDL_Renderer> renderer, const string &gameFolder,
                                         const string &title, const string &serial) {
    for (const DirEntry &entry : FileUtils::diru(gameFolder)) {
        if (FileUtils::matchExtension(entry.name, EXT_PNG))
            return IMG_LoadTexture(renderer, (gameFolder + sep + entry.name).c_str());
    }
    auto gui = Gui::getInstance();
    const string recordName = Coverdb::findRecordNameForSerial(gui->coverdb, serial);
    const string raBoxArt = ThumbnailLookup::findBoxArtPath(ThumbnailLookup::PlayStationDbName, title, recordName);
    if (!raBoxArt.empty())
        return IMG_LoadTexture(renderer, raBoxArt.c_str());
    return IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "default.png").c_str());
}

struct EditorLayout {
    SDL_Rect infoPanel;
    SDL_Rect settingsPanel;
    SDL_Rect coverRect;
    int infoTitleY = 0;
    int infoFirstRowY = 0;
    int infoRowStep = 0;
    int settingsTitleY = 0;
    int settingsFirstRowY = 0;
    int settingsRowStep = 0;
    int settingsRowHeight = 0;
    int settingsLabelX = 0;
    int settingsLabelWidth = 0;
    int settingsValueX = 0;
    int settingsValueWidth = 0;
};

EditorLayout getEditorLayout(const shared_ptr<Gui> &gui, int optionCount) {
    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    int top = gui->getContentTopY(14);
    int bottom = gui->getContentBottomY(16);
    int right = opscreen.x + opscreen.w - 18;
    int panelHeight = bottom - top;

    EditorLayout layout;
    layout.infoPanel = {opscreen.x + 18, top, editorInfoPanelWidth, panelHeight};
    layout.settingsPanel = {layout.infoPanel.x + layout.infoPanel.w + editorPanelGap, top,
                            right - (layout.infoPanel.x + layout.infoPanel.w + editorPanelGap), panelHeight};

    int coverSize = std::min(editorCoverSize, layout.infoPanel.w - (editorPanelPadding * 2));
    layout.coverRect = {layout.infoPanel.x + ((layout.infoPanel.w - coverSize) / 2), layout.infoPanel.y + 58, coverSize,
                        coverSize};
    layout.infoTitleY = layout.infoPanel.y + 14;
    layout.infoFirstRowY = layout.coverRect.y + layout.coverRect.h + 18;
    layout.infoRowStep = std::max(22, FC_GetLineHeight(gui->themeFonts[FONT_15_BOLD]) + 5);

    layout.settingsTitleY = layout.settingsPanel.y + 14;
    layout.settingsFirstRowY = layout.settingsPanel.y + 52;
    int settingsFontHeight = FC_GetLineHeight(gui->themeFonts[FONT_20_BOLD]);
    Gui::AllTextOrEmojiTokenInfo checkInfo(gui->themeFonts[FONT_20_BOLD], "|@Check|");
    int settingsContentHeight = std::max(settingsFontHeight, checkInfo.totalSize.h);
    int defaultRowStep = std::max(editorSettingsMinRowStep, settingsContentHeight + editorSettingsRowGap);
    int availableHeight =
        layout.settingsPanel.y + layout.settingsPanel.h - editorPanelPadding - layout.settingsFirstRowY;
    int availableStep = optionCount > 0 ? availableHeight / optionCount : defaultRowStep;
    layout.settingsRowStep = std::max(settingsContentHeight, std::min(defaultRowStep, availableStep));
    layout.settingsRowHeight = settingsContentHeight;
    layout.settingsLabelX = layout.settingsPanel.x + editorPanelPadding;
    layout.settingsValueWidth = editorSettingsValueWidth;
    layout.settingsValueX =
        layout.settingsPanel.x + layout.settingsPanel.w - editorPanelPadding - layout.settingsValueWidth;
    layout.settingsLabelWidth = layout.settingsValueX - layout.settingsLabelX - editorPanelPadding;
    return layout;
}

void renderCoverFrame(SDL_Shared<SDL_Renderer> renderer, const SDL_Rect &rect) {
    GuiLayout::renderRectOutline(renderer, rect, {255, 255, 255, 35});
}

void renderEditorInfoLine(const shared_ptr<Gui> &gui, const EditorLayout &layout, int row, const string &label,
                          const string &value) {
    int y = layout.infoFirstRowY + (layout.infoRowStep * row);
    int x = layout.infoPanel.x + editorPanelPadding;
    int valueX = x + editorInfoLabelWidth + 8;
    int valueWidth = layout.infoPanel.x + layout.infoPanel.w - editorPanelPadding - valueX;

    gui->renderFittedText_WithColor(gui->themeFonts[FONT_15_BOLD], label, x, y, editorInfoLabelWidth,
                                    editorInfoMaxFontSize, editorInfoMinFontSize, Gui::getTitleTextColor());
    gui->renderFittedText_WithColor(gui->themeFonts[FONT_15_BOLD], value, valueX, y, valueWidth, editorInfoMaxFontSize,
                                    editorInfoMinFontSize, editorMutedTextColor);
}

void renderCenteredFittedText(const shared_ptr<Gui> &gui, FC_Font_Shared baseFont, const string &text,
                              const SDL_Rect &rect, int maxSize, int minSize, SDL_Color color) {
    FC_Font_Shared font = gui->getFittingThemeFont(baseFont, maxSize, minSize, text, rect.w);
    Gui::AllTextOrEmojiTokenInfo textInfo(font, text);
    textInfo.setTextColor(color);
    int x = rect.x + ((rect.w - textInfo.totalSize.w) / 2);
    int y = rect.y + std::max(0, (rect.h - textInfo.totalSize.h) / 2);
    textInfo.render(x, y);
}

struct EditorOptionLine {
    EditorOptionLine(int _id, const string &_label, const string &_value) : id(_id), label(_label), value(_value) {}

    int id;
    string label;
    string value;
};

string checkedValue(bool checked) { return checked ? "|@Check|" : "|@Uncheck|"; }

string getIniValue(const Inifile &ini, const string &key) {
    auto it = ini.values.find(key);
    return it == ini.values.end() ? "" : it->second;
}

vector<EditorOptionLine> buildEditorOptions(const GuiEditor &editor) {
    vector<EditorOptionLine> options;
    options.emplace_back(OPT_FAVORITE, _("Favorite"),
                         checkedValue(editor.gameData->internal ? editor.gameData->favorite
                                                                : getIniValue(editor.gameIni, "favorite") == "1"));
    options.emplace_back(OPT_LIGHTGUN, _("Lightgun Game"),
                         checkedValue(Gui::getInstance()->lightgunGames.IsGameALightgunGame(editor.gameData)));
    options.emplace_back(OPT_PLAY_USING_RA, _("Play using RA"),
                         checkedValue(editor.gameData->internal
                                          ? editor.gameData->play_using_ra
                                          : getIniValue(editor.gameIni, "play_using_ra") == "true"));
    options.emplace_back(OPT_LOCK, _("Lock data"), checkedValue(getIniValue(editor.gameIni, "automation") == "0"));
    options.emplace_back(OPT_HIGHRES, _("High res"), checkedValue(editor.highres == 1));
    options.emplace_back(OPT_SPEEDHACK, _("SpeedHack"), checkedValue(editor.speedhack == 1));
    options.emplace_back(OPT_SCANLINES, _("Scanlines"), checkedValue(editor.scanlines == 1));
    options.emplace_back(OPT_SCANLINELV, _("Scanline Level"), to_string(editor.scanlineLevel));
    options.emplace_back(OPT_CLOCK_PSX, _("Clock"), to_string(editor.clock));
    options.emplace_back(OPT_FRAMESKIP, _("Frameskip"), to_string(editor.frameskip));
    if (!editor.internal) {
        options.emplace_back(OPT_PLUGIN, _("Plugin"), editor.gpu);
    }
    options.emplace_back(OPT_INTERPOLATION, _("Spu Interpolation"), to_string(editor.interpolation));
    return options;
}

string formatMemoryCardName(const string &memcard) {
    return memcard == "SONY" ? string(_("Internal")) : memcard + " " + "(" + _("Custom") + ")";
}

void renderEditorOptionRow(const shared_ptr<Gui> &gui, const EditorLayout &layout, const EditorOptionLine &option,
                           int visibleRow, bool selected) {
    int y = layout.settingsFirstRowY + (layout.settingsRowStep * visibleRow);
    int textY = y + ((layout.settingsRowHeight - FC_GetLineHeight(gui->themeFonts[FONT_20_BOLD])) / 2);
    if (selected) {
        string fg = gui->themeData.values["text_fg"];
        SDL_Rect selection{layout.settingsPanel.x + 8, y - 2, layout.settingsPanel.w - 16,
                           layout.settingsRowHeight + 4};
        SDL_Color selectionColor = {static_cast<Uint8>(gui->getR(fg)), static_cast<Uint8>(gui->getG(fg)),
                                    static_cast<Uint8>(gui->getB(fg)), 255};
        GuiLayout::renderRectOutline(gui->renderer, selection, selectionColor);
    }

    gui->renderFittedText(gui->themeFonts[FONT_20_BOLD], option.label, layout.settingsLabelX, textY,
                          layout.settingsLabelWidth, editorSettingsMaxFontSize, editorSettingsMinFontSize);

    int rightMargin = SCREEN_WIDTH - (layout.settingsValueX + layout.settingsValueWidth);
    Gui::AllTextOrEmojiTokenInfo valueInfo(gui->themeFonts[FONT_20_BOLD], option.value);
    valueInfo.render(rightMargin, y + ((layout.settingsRowHeight - valueInfo.totalSize.h) / 2), XALIGN_RIGHT);
}

} // namespace

namespace {
bool isEditorOptionVisible(int option, bool internal) { return !(internal && option == OPT_PLUGIN); }

int clampEditorOption(int option, bool internal) {
    if (isEditorOptionVisible(option, internal)) {
        return option;
    }

    for (int next = option + 1; next <= OPT_LAST; ++next) {
        if (isEditorOptionVisible(next, internal)) {
            return next;
        }
    }
    for (int previous = option - 1; previous >= OPT_FIRST; --previous) {
        if (isEditorOptionVisible(previous, internal)) {
            return previous;
        }
    }

    return OPT_FIRST;
}

int moveEditorOption(int option, int direction, bool internal) {
    int next = option;
    do {
        next += direction;
        if (next < OPT_FIRST) {
            return clampEditorOption(OPT_FIRST, internal);
        }
        if (next > OPT_LAST) {
            return clampEditorOption(OPT_LAST, internal);
        }
    } while (!isEditorOptionVisible(next, internal));

    return next;
}
} // namespace

//*******************************
// GuiEditor::processOptionChange
//*******************************
void GuiEditor::processOptionChange(bool direction) {
    shared_ptr<Gui> gui(Gui::getInstance());
    CfgProcessor *processor = new CfgProcessor();

    string path = gameFolder;
    if (internal) {
        path = gameData->ssFolder;
    }
    stringstream ss;
    string s;

    switch (selOption) {
    case OPT_FAVORITE:
        if (internal) {
            if (direction == true) {
                if (gameData->favorite == false) {
                    gameData->favorite = true;
                }
            } else {
                if (gameData->favorite == true) {
                    gameData->favorite = false;
                }
            }
            gui->internalDB->updateFavorite(gameData->gameId, gameData->favorite);
        } else {
            if (gameIni.values["favorite"] == "")
                gameIni.values["favorite"] = "0"; // doesn't exist yet in this ini so set to 0
            if (direction == true) {
                if (gameIni.values["favorite"] == "0") {
                    gameIni.values["favorite"] = "1";
                }
            } else {
                if (gameIni.values["favorite"] == "1") {
                    gameIni.values["favorite"] = "0";
                }
            }
            gameIni.save(gameIni.path);
        }
        break;

    case OPT_LIGHTGUN: {
        auto &lightgunGames = Gui::getInstance()->lightgunGames;
        bool isLightgun = lightgunGames.IsGameALightgunGame(gameData);
        bool changeRA = false;

        if (direction == true) {
            if (isLightgun == false) {
                lightgunGames.AddGame(gameData);
                isLightgun = true;
                gameData->play_using_ra = true;
                changeRA = true;
            }
        } else {
            if (isLightgun == true) {
                lightgunGames.RemoveGame(gameData);
                isLightgun = false;
                gameData->play_using_ra = false;
                changeRA = true;
            }
        }

        if (changeRA) {
            gameData->play_using_ra = isLightgun;
            if (internal)
                gui->internalDB->updatePlayUsingRA(gameData->gameId, gameData->play_using_ra);
            else {
                gameIni.values["play_using_ra"] = (isLightgun) ? "true" : "false";
                gameIni.save(gameIni.path);
            }
        }
    } break;

    case OPT_PLAY_USING_RA: {
        auto &lightgunGames = Gui::getInstance()->lightgunGames;
        bool isLightgun = lightgunGames.IsGameALightgunGame(gameData);
        if (isLightgun)
            break; // islightgun setting is on you can't change using_ra.

        if (internal) {
            if (direction == true) {
                if (gameData->play_using_ra == false) {
                    gameData->play_using_ra = true;
                }
            } else {
                if (gameData->play_using_ra == true) {
                    gameData->play_using_ra = false;
                }
            }
            gui->internalDB->updatePlayUsingRA(gameData->gameId, gameData->play_using_ra);
        } else {
            if (gameIni.values["play_using_ra"] == "")
                gameIni.values["play_using_ra"] = "false"; // doesn't exist yet in this ini so set to 0
            if (direction == true) {
                if (gameIni.values["play_using_ra"] == "false") {
                    gameIni.values["play_using_ra"] = "true";
                }
            } else {
                if (gameIni.values["play_using_ra"] == "true") {
                    gameIni.values["play_using_ra"] = "false";
                }
            }
            gameIni.save(gameIni.path);
        }
    } break;

    case OPT_LOCK:
        if (!internal) {
            if (direction == true) {
                if (gameIni.values["automation"] == "1") {
                    gameIni.values["automation"] = "0";
                }
            } else {
                if (gameIni.values["automation"] == "0") {
                    gameIni.values["automation"] = "1";
                }
            }
            gameIni.save(gameIni.path);
        }
        break;

    case OPT_HIGHRES:
        if (direction == false) {
            if (highres == 1) {
                highres = 0;
            }
        } else {
            if (highres == 0) {
                highres = 1;
            }
        }
        gameIni.values["highres"] = to_string(highres);

        processor->replace(gameIni.entry, path, "gpu_neon.enhancement_enable",
                           "gpu_neon.enhancement_enable = " + gameIni.values["highres"], internal);
        if (!internal) {
            gameIni.save(gameIni.path);
        }

        refreshData();
        break;

    case OPT_SPEEDHACK:
        if (direction == false) {
            if (speedhack == 1) {
                speedhack = 0;
            }
        } else {
            if (speedhack == 0) {
                speedhack = 1;
            }
        }

        processor->replace(gameIni.entry, path, "gpu_neon.enhancement_no_main",
                           "gpu_neon.enhancement_no_main = " + to_string(speedhack), internal);
        refreshData();
        break;

    case OPT_SCANLINES:
        if (direction == false) {
            if (scanlines == 1) {
                scanlines = 0;
            }
        } else {
            if (scanlines == 0) {
                scanlines = 1;
            }
        }
        processor->replace(gameIni.entry, path, "scanlines", "scanlines = " + to_string(scanlines), internal);
        refreshData();
        break;

    case OPT_SCANLINELV:
        if (direction == true) {
            scanlineLevel++;
            if (scanlineLevel > 100) {
                scanlineLevel = 100;
            }
        } else {
            scanlineLevel--;
            if (scanlineLevel < 0) {
                scanlineLevel = 0;
            }
        }

        ss << std::hex << scanlineLevel;
        s = ss.str();

        processor->replace(gameIni.entry, path, "scanline_level", "scanline_level = " + s, internal);
        refreshData();
        break;

    case OPT_CLOCK_PSX:
        if (direction == true) {
            clock++;
            if (clock > 100) {
                clock = 100;
            }

        } else {
            clock--;
            if (clock < 0) {
                clock = 0;
            }
        }

        ss << std::hex << clock;
        s = ss.str();

        processor->replace(gameIni.entry, path, "psx_clock", "psx_clock = " + s, internal);
        refreshData();
        break;

    case OPT_FRAMESKIP:
        if (direction == true) {

            frameskip++;
            if (frameskip > 3) {
                frameskip = 3;
            }
        } else {
            frameskip--;
            if (frameskip < 0) {
                frameskip = 0;
            }
        }

        ss << std::hex << frameskip;
        s = ss.str();

        processor->replace(gameIni.entry, path, "frameskip3", "frameskip3 = " + s, internal);
        refreshData();
        break;

    case OPT_INTERPOLATION:
        if (direction == true) {
            interpolation++;
            if (interpolation > 3) {
                interpolation = 3;
            }
        } else {
            interpolation--;
            if (interpolation < 0) {
                interpolation = 0;
            }
        }

        ss << std::hex << interpolation;
        s = ss.str();

        processor->replace(gameIni.entry, path, "spu_config.iUseInterpolation", "spu_config.iUseInterpolation = " + s,
                           internal);
        refreshData();
        break;

    case OPT_PLUGIN:
        if (!internal) {
            if (direction == true) {
                gpu = "gpu_peops.so";
            } else {
                gpu = "builtin_gpu";
            }
            processor->replace(gameIni.entry, path, "Gpu3", "Gpu3 = " + gpu, internal);
            refreshData();
        }
        break;
    }
    delete (processor);
}

//*******************************
// GuiEditor::refreshData
//*******************************
void GuiEditor::refreshData() {
    shared_ptr<Gui> gui(Gui::getInstance());
    CfgProcessor *processor = new CfgProcessor();
    string path = gameFolder;
    if (internal) {
        path = gameData->ssFolder;
    }
    highres = atoi(processor->getValue(path, "gpu_neon.enhancement_enable").c_str());
    speedhack = atoi(processor->getValue(path, "gpu_neon.enhancement_no_main").c_str());
    clock = strtol(processor->getValue(path, "psx_clock").c_str(), nullptr, 16);
    gpu = processor->getValue(path, "gpu3");
    frameskip = atoi(processor->getValue(path, "frameskip3").c_str());
    dither = atoi(processor->getValue(path, "gpu_peops.iUseDither").c_str());
    scanlines = atoi(processor->getValue(path, "scanlines").c_str());
    scanlineLevel = strtol(processor->getValue(path, "scanline_level").c_str(), nullptr, 16);
    interpolation = strtol(processor->getValue(path, "spu_config.iUseInterpolation").c_str(), nullptr, 16);

    delete processor;
}

//*******************************
// GuiEditor::init
//*******************************
void GuiEditor::init() {
    shared_ptr<Gui> gui(Gui::getInstance());
    if (!internal) {
        if (this->gameIni.values["memcard"] != "SONY") {
            string cardpath = Env::getPathToMemCardsDir() + sep + this->gameIni.values["memcard"];
            if (!FileUtils::exists(cardpath)) {
                this->gameIni.values["memcard"] = "SONY";
            }
        }

        cover = loadCoverTexture(renderer, gameFolder, gameData->title, gameData->serial);
    } else {
        // recover ini
        this->gameIni.values["title"] = gameData->title;
        this->gameIni.values["publisher"] = gameData->publisher;
        this->gameIni.values["year"] = to_string(gameData->year);
        this->gameIni.values["players"] = to_string(gameData->players);
        this->gameIni.values["memcard"] = gameData->memcard;

        if (this->gameIni.values["memcard"] != "SONY") {
            string cardpath = Env::getPathToMemCardsDir() + sep + this->gameIni.values["memcard"];
            if (!FileUtils::exists(cardpath)) {
                this->gameIni.values["memcard"] = "SONY";
            }
        }

        cover = loadCoverTexture(renderer, gameData->folder, gameData->title, gameData->serial);
    }

    refreshData();
}

//*******************************
// GuiEditor::render
//*******************************
void GuiEditor::render() {
    shared_ptr<Gui> gui(Gui::getInstance());

    gui->renderBackground();
    gui->renderTextBar();

    vector<EditorOptionLine> options = buildEditorOptions(*this);
    selOption = clampEditorOption(selOption, internal);
    EditorLayout layout = getEditorLayout(gui, options.size());

    GuiLayout::renderPanel(renderer, layout.infoPanel, editorPanelFillColor, editorPanelBorderColor);
    GuiLayout::renderPanel(renderer, layout.settingsPanel, editorPanelFillColor, editorPanelBorderColor);

    SDL_Rect titleRect{layout.infoPanel.x + editorPanelPadding, layout.infoTitleY,
                       layout.infoPanel.w - (editorPanelPadding * 2), 30};
    renderCenteredFittedText(gui, gui->themeFonts[FONT_20_BOLD], gameIni.values["title"], titleRect,
                             editorTitleMaxFontSize, editorTitleMinFontSize, Gui::getTitleTextColor());

    GuiLayout::renderTextureFit(renderer, cover, layout.coverRect);
    renderCoverFrame(renderer, layout.coverRect);

    string folderText = internal ? gameData->folder : gameIni.entry;
    renderEditorInfoLine(gui, layout, 0, _("Folder"), folderText);
    renderEditorInfoLine(gui, layout, 1, _("Published by"), gameIni.values["publisher"]);
    renderEditorInfoLine(gui, layout, 2, _("Year"), gameIni.values["year"]);
    renderEditorInfoLine(gui, layout, 3, _("Players"), gameIni.values["players"]);
    renderEditorInfoLine(gui, layout, 4, _("Memory Card"), formatMemoryCardName(gameIni.values["memcard"]));

    SDL_Rect settingsTitleRect{layout.settingsPanel.x + editorPanelPadding, layout.settingsTitleY,
                               layout.settingsPanel.w - (editorPanelPadding * 2), 28};
    renderCenteredFittedText(gui, gui->themeFonts[FONT_20_BOLD], _("Options"), settingsTitleRect,
                             editorSettingsTitleMaxFontSize, editorSettingsTitleMinFontSize, Gui::getTitleTextColor());

    for (int i = 0; i < options.size(); ++i) {
        renderEditorOptionRow(gui, layout, options[i], i, options[i].id == selOption);
    }

    string guiMenu = "|@T| " + _("Rename");

    if (!internal) {
        guiMenu += "  |@S| " + _("Change MC") + " ";

        if (gameIni.values["memcard"] == "SONY") {
            guiMenu += "|@Start| " + _("Share MC") + "  ";
        }
    }

    guiMenu += " |@O| " + _("Go back") + "|";

    gui->renderStatus(guiMenu);

    SDL_RenderPresent(renderer);
}

//*******************************
// GuiEditor::loop
//*******************************
void GuiEditor::loop() {
    shared_ptr<Gui> gui(Gui::getInstance());
    menuVisible = true;
    while (menuVisible) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            gui->mapper.handleHotPlug(&e);
            gui->mapper.handlePowerBtn(&e);
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.scancode == SDL_SCANCODE_SLEEP || e.key.keysym.sym == SDLK_ESCAPE) {
                    gui->drawText(_("POWERING OFF... PLEASE WAIT"));
                    System::shutdown();
                }
            }
            // this is for pc Only
            if (e.type == SDL_QUIT) {
                menuVisible = false;
            }
            switch (e.type) {
            case SDL_CONTROLLERHATMOTIONDOWN: /* Handle Joystick Motion */
            case SDL_CONTROLLERHATMOTIONUP:

                if (gui->mapper.isDown(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        selOption = moveEditorOption(selOption, 1, internal);
                        render();
                    } while (fastForwardUntilAnotherEvent(120));
                }
                if (gui->mapper.isUp(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        selOption = moveEditorOption(selOption, -1, internal);
                        render();
                    } while (fastForwardUntilAnotherEvent(120));
                }

                if (gui->mapper.isRight(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        processOptionChange(true);
                        render();
                    } while (fastForwardUntilAnotherEvent(80));
                }
                if (gui->mapper.isLeft(&e)) {
                    do {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        processOptionChange(false);
                        render();
                    } while (fastForwardUntilAnotherEvent(80));
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                if (!internal) {
                    if (gameIni.values["memcard"] == "SONY") {
                        if (e.cbutton.button == SDL_BTN_START) {
                            Mix_PlayChannel(-1, gui->cursor, 0);
                            GuiKeyboard *keyboard = new GuiKeyboard(renderer);
                            keyboard->label = _("Enter new name for memory card");
                            keyboard->result = gameIni.values["title"];
                            keyboard->show();
                            string result = keyboard->result;
                            bool cancelled = keyboard->cancelled;
                            delete (keyboard);

                            if (result.empty()) {
                                cancelled = true;
                            }

                            if (!cancelled) {
                                Memcard *memcard = new Memcard(gui->pathToGamesDir);
                                string savePath =
                                    Env::getPathToSaveStatesDir() + sep + gameIni.entry + sep + "memcards";
                                memcard->storeToRepo(savePath, result);
                                gameIni.values["memcard"] = result;
                                gameIni.save(gameIni.path);
                            }
                        };
                    }
                } else {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                }

                if (e.cbutton.button == SDL_BTN_SQUARE) {
                    if (!internal) {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        GuiSelectMemcard *selector = new GuiSelectMemcard(renderer);
                        selector->cardSelected = gameIni.values["memcard"];
                        selector->show();

                        if (selector->selected != -1) {
                            if (selector->selected == 0) {
                                gameIni.values["memcard"] = "SONY";
                                gameIni.save(gameIni.path);
                            } else {
                                gameIni.values["memcard"] = selector->cards[selector->selected];
                                gameIni.save(gameIni.path);
                            }
                        }
                        delete (selector);
                    } else {
                        Mix_PlayChannel(-1, gui->cancel, 0);
                    }
                };

                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    cover = nullptr;
                    menuVisible = false;
                };

                if (e.cbutton.button == SDL_BTN_TRIANGLE) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    GuiKeyboard *keyboard = new GuiKeyboard(renderer);
                    keyboard->label = _("Enter new game name");
                    keyboard->result = gameIni.values["title"];
                    keyboard->show();
                    string result = keyboard->result;
                    bool cancelled = keyboard->cancelled;
                    delete (keyboard);

                    if (result.empty()) {
                        cancelled = true;
                    }

                    if (!cancelled) {
                        if (!internal) {
                            gameIni.values["title"] = result;
                            gameIni.values["automation"] = "0";
                            gameIni.save(gameIni.path);
                            changes = true;
                        } else {
                            lastName = result;
                            changes = true;
                        }
                    }
                    refreshData();
                };
            }
        }
        render();
    }
}
