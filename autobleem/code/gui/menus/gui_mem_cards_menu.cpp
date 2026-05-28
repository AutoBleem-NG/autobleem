#include "gui_mem_cards_menu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <string>
#include "../gui.h"
#include "../../engine/card_edit.h"
#include "../../engine/mem_card.h"
#include "../gui_confirm.h"
#include "../gui_keyboard.h"
#include "../../lang.h"
#include "../../utils/file_utils.h"
#include "../../utils/string_utils.h"

using namespace std;

namespace {
const int memcardsPanelPadding = 10;
const int memcardsTitleGap = 8;
const int memcardsHeaderRow = 2;
const int memcardsFirstListRow = 4;
const int memcardsMaxVisible = 13;
const int memcardsNameX = 0;
const int memcardsCard1X = 420;
const int memcardsCard2X = 545;
const int memcardsSavesX = 670;
const int memcardsFreeX = 760;
const int memcardsDetailsX = 865;
const int memcardsDetailsWidth = 350;
const int memcardsColumnGap = 14;
const int memcardsHeaderLineGap = 5;
const int memcardsHeaderMinLineGap = 8;
const int memcardsDetailTitleGap = 6;
const int memcardsDetailMaxTitles = 8;
const int memcardsSmallFontSize = 15;
const int memcardsMinFontSize = 12;
const SDL_Color memcardsHeaderColor = {80, 170, 255, 255};
const int memcardsHeaderAlpha = 95;
const char customMemcardsDir[] = "!MemCards";
const char card1File[] = "card1.mcd";
const char card2File[] = "card2.mcd";

string formatCardUsage(const GuiMemcards::CardFileSummary &summary) {
    if (!summary.exists) {
        return "-";
    }

    return to_string(summary.usedSlots) + "/15";
}

string formatTotalSaves(const GuiMemcards::CustomCardSummary &summary) {
    return to_string(summary.card1.saves + summary.card2.saves);
}

string formatTotalFree(const GuiMemcards::CustomCardSummary &summary) {
    return to_string(summary.card1.freeSlots + summary.card2.freeSlots);
}

int getListY(FC_Font_Shared font, int yoffset, int row) { return yoffset + (FC_GetLineHeight(font) * row); }
} // namespace

//*******************************
// GuiMemcards::init
//*******************************
void GuiMemcards::init() {
    GuiMenuBase::init(); // call the base init
    firstRow = memcardsFirstListRow;
    maxVisible = std::max(1, std::min(memcardsMaxVisible, atoi(gui->themeData.values["lines"].c_str())));
    refreshCards();
}

void GuiMemcards::refreshCards() {
    summaries.clear();

    Memcard *memcardOps = new Memcard(gui->pathToGamesDir);
    lines = memcardOps->list();
    delete memcardOps;

    for (const string &cardName : lines) {
        string cardPath = gui->pathToGamesDir + sep + customMemcardsDir + sep + cardName;
        CustomCardSummary summary;
        summary.name = cardName;
        summary.card1 = readCardFileSummary(renderer, cardPath + sep + card1File);
        summary.card2 = readCardFileSummary(renderer, cardPath + sep + card2File);
        summaries.push_back(summary);
    }

    if (lines.empty()) {
        selected = 0;
    } else if (selected >= getVerticalSize()) {
        selected = getVerticalSize() - 1;
    } else if (selected < 0) {
        selected = 0;
    }
    computePagePosition();
    firstRender = false;
}

GuiMemcards::CardFileSummary GuiMemcards::readCardFileSummary(SDL_Shared<SDL_Renderer> renderer, const string &path) {
    CardFileSummary summary;
    if (!FileUtils::exists(path)) {
        return summary;
    }

    summary.exists = true;
    CardEdit card(renderer);
    card.load_file(path);
    for (int slot = 0; slot < 15; slot++) {
        if (card.get_slot_is_used(slot)) {
            summary.usedSlots++;
        }
        if (card.get_slot_is_free(slot)) {
            summary.freeSlots++;
        }
        if (card.is_slot_top(slot)) {
            summary.saves++;
            summary.saveTitles.push_back(card.get_slot_title(slot));
        }
    }

    return summary;
}

void GuiMemcards::render() {
    SDL_RenderClear(renderer);
    gui->renderBackground();
    gui->renderTextBar();
    yoffset = gui->getContentTopY(memcardsPanelPadding) + memcardsTitleGap;

    gui->renderTitleLine(getTitle(), 0, yoffset);
    renderColumnHeaders();
    renderLines();
    renderSelectionBox();

    gui->renderStatus(getStatusLine());
    SDL_RenderPresent(renderer);
}

void GuiMemcards::renderColumnHeaders() {
    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    int y = getListY(font, yoffset, memcardsHeaderRow);
    int rowHeight = FC_GetLineHeight(font);
    int nameX = opscreen.x + 10 + memcardsNameX;
    int card1X = opscreen.x + 10 + memcardsCard1X;
    int card2X = opscreen.x + 10 + memcardsCard2X;
    int savesX = opscreen.x + 10 + memcardsSavesX;
    int freeX = opscreen.x + 10 + memcardsFreeX;

    gui->renderFittedText_WithColor(font, _("Card"), nameX, y, memcardsCard1X - memcardsColumnGap,
                                    memcardsSmallFontSize, memcardsMinFontSize, memcardsHeaderColor);
    gui->renderFittedText_WithColor(font, _("Card") + " 1", card1X, y, memcardsCard2X - memcardsCard1X,
                                    memcardsSmallFontSize, memcardsMinFontSize, memcardsHeaderColor);
    gui->renderFittedText_WithColor(font, _("Card") + " 2", card2X, y, memcardsSavesX - memcardsCard2X,
                                    memcardsSmallFontSize, memcardsMinFontSize, memcardsHeaderColor);
    gui->renderFittedText_WithColor(font, "Saves", savesX, y, memcardsFreeX - memcardsSavesX, memcardsSmallFontSize,
                                    memcardsMinFontSize, memcardsHeaderColor);
    gui->renderFittedText_WithColor(font, _("Free"), freeX, y, memcardsDetailsX - memcardsFreeX + memcardsDetailsWidth,
                                    memcardsSmallFontSize, memcardsMinFontSize, memcardsHeaderColor);

    SDL_SetRenderDrawColor(renderer, memcardsHeaderColor.r, memcardsHeaderColor.g, memcardsHeaderColor.b,
                           memcardsHeaderAlpha);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int lineY =
        std::min(y + rowHeight + memcardsHeaderLineGap, getListY(font, yoffset, firstRow) - memcardsHeaderMinLineGap);
    SDL_RenderDrawLine(renderer, opscreen.x + 10, lineY, opscreen.x + opscreen.w - 10, lineY);
}

void GuiMemcards::renderLineIndexOnRow(int index, int row) {
    if (index < 0 || index >= summaries.size()) {
        return;
    }

    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    const CustomCardSummary &summary = summaries[index];
    int y = getListY(font, yoffset, row);
    int nameX = opscreen.x + 10 + memcardsNameX;
    int card1X = opscreen.x + 10 + memcardsCard1X;
    int card2X = opscreen.x + 10 + memcardsCard2X;
    int savesX = opscreen.x + 10 + memcardsSavesX;
    int freeX = opscreen.x + 10 + memcardsFreeX;

    gui->renderFittedText(font, summary.name, nameX, y, memcardsCard1X - memcardsColumnGap, memcardsSmallFontSize,
                          memcardsMinFontSize);
    gui->renderFittedText(font, formatCardUsage(summary.card1), card1X, y, memcardsCard2X - memcardsCard1X,
                          memcardsSmallFontSize, memcardsMinFontSize);
    gui->renderFittedText(font, formatCardUsage(summary.card2), card2X, y, memcardsSavesX - memcardsCard2X,
                          memcardsSmallFontSize, memcardsMinFontSize);
    gui->renderFittedText(font, formatTotalSaves(summary), savesX, y, memcardsFreeX - memcardsSavesX,
                          memcardsSmallFontSize, memcardsMinFontSize);
    gui->renderFittedText(font, formatTotalFree(summary), freeX, y,
                          memcardsDetailsX - memcardsFreeX + memcardsDetailsWidth, memcardsSmallFontSize,
                          memcardsMinFontSize);
}

void GuiMemcards::renderSelectedCardDetails() {
    if (selected < 0 || selected >= summaries.size()) {
        return;
    }

    SDL_Rect opscreen = gui->getOpscreenRectOfTheme();
    const CustomCardSummary &summary = summaries[selected];
    int x = opscreen.x + 10 + memcardsDetailsX;
    int visibleRows = 0;
    if (firstVisibleIndex < getVerticalSize()) {
        visibleRows = std::min(maxVisible, getVerticalSize() - firstVisibleIndex);
    }
    int y = getListY(font, yoffset, firstRow + visibleRows) + memcardsDetailTitleGap;
    int rowHeight = FC_GetLineHeight(font);
    int maxY = gui->getContentBottomY(memcardsPanelPadding);
    int line = 0;

    auto renderDetail = [&](const string &label, const vector<string> &titles) {
        if (y + (rowHeight * (line + 1)) > maxY || line >= memcardsDetailMaxTitles) {
            return;
        }
        if (titles.empty()) {
            gui->renderFittedText(font, label + ": -", x, y + (rowHeight * line), memcardsDetailsWidth,
                                  memcardsSmallFontSize, memcardsMinFontSize);
            line++;
            return;
        }

        for (size_t i = 0; i < titles.size(); i++) {
            if (y + (rowHeight * (line + 1)) > maxY || line >= memcardsDetailMaxTitles) {
                break;
            }
            string text = i == 0 ? label + ": " + titles[i] : "  " + titles[i];
            gui->renderFittedText(font, text, x, y + (rowHeight * line), memcardsDetailsWidth, memcardsSmallFontSize,
                                  memcardsMinFontSize);
            line++;
        }
    };

    renderDetail(_("Card") + " 1", summary.card1.saveTitles);
    renderDetail(_("Card") + " 2", summary.card2.saveTitles);
}

//*******************************
// GuiMemcards::getStatusLine
//*******************************
// returns the status line at the bottom
string GuiMemcards::getStatusLine() {
    int displayIndex = lines.empty() ? 0 : selected + 1;
    return _("Card") + " " + to_string(displayIndex) + "/" + to_string(getVerticalSize()) + "   |@L1|/|@R1| " +
           _("Page") + "   |@X| " + _("Rename") + "  |@S| " + _("New Card") + "   |@T| " + _("Delete") + "  |@O| " +
           _("Go back") + "|";
}

//*******************************
// GuiMemcards::doCirclePressed
//*******************************
void GuiMemcards::doCircle_Pressed() {
    Mix_PlayChannel(-1, gui->cancel, 0);
    menuVisible = false;
}

//*******************************
// GuiMemcards::doSquarePressed
//*******************************
void GuiMemcards::doSquare_Pressed() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    GuiKeyboard *keyboard = new GuiKeyboard(renderer);
    keyboard->label = _("Enter new card name");
    keyboard->show();
    string result = keyboard->result;
    bool cancelled = keyboard->cancelled;
    delete (keyboard);

    if (result.empty()) {
        cancelled = true;
    }

    string testResult = result;
    if (StringUtils::compareCaseInsensitive("sony", testResult)) {
        cancelled = true;
    }

    if (!cancelled) {
        Memcard *memcardOps = new Memcard(gui->pathToGamesDir);
        memcardOps->newCard(result);
        delete memcardOps;
        refreshCards();
        int i = 0;
        for (const string &card : lines) {
            if (card == result) {
                selected = i;
                firstVisibleIndex = i;
                lastVisibleIndex = firstVisibleIndex + maxVisible - 1;

                if (getVerticalSize() > maxVisible) {
                    if (lastVisibleIndex >= getVerticalSize()) {
                        lastVisibleIndex = getVerticalSize() - 1;
                        firstVisibleIndex = lastVisibleIndex - maxVisible + 1;
                    }
                }
            }
            i++;
        }
    }
    render();
}

//*******************************
// GuiMemcards::doTrianglePressed
//*******************************
void GuiMemcards::doTriangle_Pressed() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    if (getVerticalSize() != 0) {
        GuiConfirm *guiConfirm = new GuiConfirm(renderer);
        guiConfirm->label = _("Delete card") + " '" + lines[selected] + "' ?";
        guiConfirm->show();
        bool result = guiConfirm->result;
        delete (guiConfirm);

        if (result) {
            Memcard *memcardOps = new Memcard(gui->pathToGamesDir);
            memcardOps->deleteCard(lines[selected]);
            delete memcardOps;
            refreshCards();
            if (selected >= getVerticalSize()) {
                selected = getVerticalSize() - 1;
            }
            computePagePosition();
        }
        render();
    }
}

//*******************************
// GuiMemcards::doCrossPressed
//*******************************
void GuiMemcards::doCross_Pressed() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    if (lines.empty()) {
        return;
    }

    GuiKeyboard *keyboard = new GuiKeyboard(renderer);
    keyboard->label = _("Enter new name for card") + " '" + lines[selected] + "'";
    keyboard->result = lines[selected];
    keyboard->show();
    string result = keyboard->result;
    bool cancelled = keyboard->cancelled;
    delete (keyboard);

    if (result.empty()) {
        cancelled = true;
    }

    string testResult = result;
    if (StringUtils::compareCaseInsensitive("sony", testResult)) {
        cancelled = true;
    }

    for (const string &card : lines) {
        if (card == result) {
            // orevent overwrite other card
            cancelled = true;
        }
    }

    if (!cancelled) {
        Memcard *memcardOps = new Memcard(gui->pathToGamesDir);
        memcardOps->rename(lines[selected], result);
        delete memcardOps;
        init();
        int pos = 0;
        for (const string &card : lines) {
            if (card == result) {
                selected = pos;
                firstVisibleIndex = pos;
                lastVisibleIndex = firstVisibleIndex + maxVisible - 1;
            }
            pos++;
        }
    }
    render();
}
