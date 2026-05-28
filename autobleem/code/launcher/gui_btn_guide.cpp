//
// Created by screemer on 2019-03-02.
//

#include "gui_btn_guide.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <string>
#include <vector>
#include "../gui/gui.h"
#include "../gui/gui_text.h"
#include "../lang.h"
#include "../engine/scanner.h"
#include "../system/process_utils.h"

using namespace std;

namespace {
const int buttonGuideTitleMaxFontSize = 18;
const int buttonGuideTitleMinFontSize = 12;
const int buttonGuideColumnMaxFontSize = 22;
const int buttonGuideColumnMinFontSize = 14;
const int buttonGuideColumnGap = 42;
const int buttonGuideColumnInset = 24;
const int buttonGuideTextGap = 12;
const int buttonGuideBottomPadding = 68;
const int buttonGuideHeaderGap = 14;
const int buttonGuideTitleToContentGap = 18;
const int buttonGuideHeaderLineGap = 4;
const int buttonGuideHeaderLineAlpha = 95;
const int buttonGuideMinRowGap = 12;
const SDL_Color buttonGuideGroupTitleColor = {80, 170, 255, 255};

struct ButtonGuideColumn {
    string title;
    vector<GuiText::ColumnLine> lines;
};

int getGuideRowStep(FC_Font_Shared font) {
    int lineHeight = FC_GetLineHeight(font);
    return lineHeight + max(buttonGuideMinRowGap, lineHeight / 2);
}

int getGuideColumnHeight(FC_Font_Shared font, const ButtonGuideColumn &column) {
    int height = FC_GetLineHeight(font) + buttonGuideHeaderGap;
    int rowStep = getGuideRowStep(font);
    for (const auto &line : column.lines) {
        if (line.first == "" && line.second == "") {
            height += GuiText::getColumnGroupGap(font);
        } else if (line.first == "") {
            height += rowStep + buttonGuideHeaderGap;
        } else {
            height += rowStep;
        }
    }

    return height;
}

int getGuideKeyWidth(FC_Font_Shared font, const vector<GuiText::ColumnLine> &lines) {
    int width = 0;
    for (const auto &line : lines) {
        Gui::AllTextOrEmojiTokenInfo keyInfo(font, line.first);
        width = max(width, keyInfo.totalSize.w);
    }

    return width;
}

bool guideColumnFits(FC_Font_Shared font, const ButtonGuideColumn &column, int columnWidth, int maxHeight) {
    if (getGuideColumnHeight(font, column) > maxHeight) {
        return false;
    }

    Gui::AllTextOrEmojiTokenInfo titleInfo(font, column.title);
    if (titleInfo.totalSize.w > columnWidth) {
        return false;
    }

    int keyWidth = getGuideKeyWidth(font, column.lines);
    int textWidth = columnWidth - keyWidth - buttonGuideTextGap;
    for (const auto &line : column.lines) {
        Gui::AllTextOrEmojiTokenInfo keyInfo(font, line.first);
        Gui::AllTextOrEmojiTokenInfo textInfo(font, line.second);
        if (line.first == "" && line.second == "") {
            continue;
        }
        if (line.first == "" && textInfo.totalSize.w > columnWidth) {
            return false;
        }
        if (line.first != "" && (keyInfo.totalSize.w > keyWidth || textInfo.totalSize.w > textWidth)) {
            return false;
        }
    }

    return true;
}

FC_Font_Shared getFittingGuideColumnFont(const shared_ptr<Gui> &gui, const ButtonGuideColumn &column, int columnWidth,
                                         int maxHeight) {
    FC_Font_Shared baseFont = gui->themeFonts[FONT_22_MED];
    FontType fontType = GuiText::getFontTypeForThemeFont(gui.get(), baseFont);
    for (int size = buttonGuideColumnMaxFontSize; size >= buttonGuideColumnMinFontSize; size--) {
        FC_Font_Shared font = size == buttonGuideColumnMaxFontSize
                                  ? baseFont
                                  : gui->sizesOfThemeFont.GetFont(size, gui->themeFonts, fontType);
        if (guideColumnFits(font, column, columnWidth, maxHeight)) {
            return font;
        }
    }

    return gui->sizesOfThemeFont.GetFont(buttonGuideColumnMinFontSize, gui->themeFonts, fontType);
}

int renderGuideHeader(const shared_ptr<Gui> &gui, const string &title, int x, int y, int width) {
    int height =
        gui->renderFittedText_WithColor(gui->themeFonts[FONT_20_BOLD], title, x, y, width, buttonGuideTitleMaxFontSize,
                                        buttonGuideTitleMinFontSize, buttonGuideGroupTitleColor);

    SDL_SetRenderDrawColor(gui->renderer, buttonGuideGroupTitleColor.r, buttonGuideGroupTitleColor.g,
                           buttonGuideGroupTitleColor.b, buttonGuideHeaderLineAlpha);
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawLine(gui->renderer, x, y + height + buttonGuideHeaderLineGap, x + width,
                       y + height + buttonGuideHeaderLineGap);
    return height + buttonGuideHeaderGap;
}

void renderGuideColumn(const shared_ptr<Gui> &gui, FC_Font_Shared font, const ButtonGuideColumn &column, int x, int y,
                       int width) {
    int keyWidth = getGuideKeyWidth(font, column.lines);
    int textX = x + keyWidth + buttonGuideTextGap;
    int rowStep = getGuideRowStep(font);
    int groupGap = GuiText::getColumnGroupGap(font);
    int currentY = y;

    currentY += renderGuideHeader(gui, column.title, x, currentY, width);

    for (const auto &guideLine : column.lines) {
        if (guideLine.first != "" && guideLine.second != "") {
            Gui::AllTextOrEmojiTokenInfo keyInfo(font, guideLine.first);
            Gui::AllTextOrEmojiTokenInfo textInfo(font, guideLine.second);
            int rowHeight = max(keyInfo.totalSize.h, textInfo.totalSize.h);
            keyInfo.render(x, currentY + ((rowHeight - keyInfo.totalSize.h) / 2));
            textInfo.render(textX,
                            currentY + ((rowHeight - textInfo.totalSize.h) / 2) + Gui::getTextVisualYOffset(font));
            currentY += rowStep;
        } else if (guideLine.second != "") {
            currentY += renderGuideHeader(gui, guideLine.second, x, currentY, width);
        } else {
            currentY += groupGap;
        }
    }
}
} // namespace

//*******************************
// GuiBtnGuide::render
//*******************************
void GuiBtnGuide::render() {
    shared_ptr<Gui> gui(Gui::getInstance());
    gui->renderBackground();

    gui->renderTextBar();
    int yoffset = gui->getContentTopY();
    int titleHeight = gui->renderTitleLine(_("Button Guide"), 0, yoffset);
    int contentY = yoffset + titleHeight + buttonGuideTitleToContentGap;

    vector<ButtonGuideColumn> columns = {
        {_("Game Browser"),
         {
             {"|@X| / |@O|", _("Select or cancel highlighted option")},
             {"|@S|", _("Run using RetroArch")},
             {"|@R1| / |@L1|", _("Quick scroll to next letter")},
             {"|@Start|", _("Random Game")},
             {"|@Select|", _("Next Game Platform")},
             {"|@L2| + |@Select|", _("Change USB Games Sub-Directory")},
             {"|@L2| + |@Select|", _("Change RetroArch Playlist")},
         }},
        {_("In Game"),
         {
             {"|@Select| + |@Start|", _("Emulator config MENU")},
             {_("RESET"), _("Quit emulation - back to AutoBleem")},
             {"", ""},
             {"", _("In RetroArch Game")},
             {"|@Select| + |@Start|", _("Open RetroArch Menu")},
             {_("POWER"), _("Exit to EvoUI")},
             {"", ""},
             {"", _("In Boot Menu")},
             {"|@L2| + |@R2|", _("Safe Power Off The Console")},
         }},
    };

    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    int columnWidth = (opscreen.w - buttonGuideColumnGap - (buttonGuideColumnInset * 2)) / 2;
    int leftX = opscreen.x + buttonGuideColumnInset;
    int rightX = leftX + columnWidth + buttonGuideColumnGap;
    int maxHeight = SCREEN_HEIGHT - contentY - buttonGuideBottomPadding;

    FC_Font_Shared leftFont = getFittingGuideColumnFont(gui, columns[0], columnWidth, maxHeight);
    FC_Font_Shared rightFont = getFittingGuideColumnFont(gui, columns[1], columnWidth, maxHeight);

    renderGuideColumn(gui, leftFont, columns[0], leftX, contentY, columnWidth);
    renderGuideColumn(gui, rightFont, columns[1], rightX, contentY, columnWidth);

    gui->renderStatus("|@O| " + _("Go back") + "|");
    SDL_RenderPresent(renderer);
}

//*******************************
// GuiBtnGuide::loop
//*******************************
void GuiBtnGuide::loop() {
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
            case SDL_CONTROLLERBUTTONUP:
                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    menuVisible = false;
                };
            }
        }
    }
}
