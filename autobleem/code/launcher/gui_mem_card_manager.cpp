//
// Created by screemer on 2019-07-30.
//

#include "gui_mem_card_manager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include <vector>
#include "../log.h"
#include "../gui/gui.h"
#include "../lang.h"
#include "../gui/gui_confirm.h"
#include "../gui/gui_select_mem_card.h"
#include "../environment.h"
#include "../system/process_utils.h"

namespace {
const int memoryCardSlotCount = 15;
const int memoryCardInfoWidth = 500;
const int memoryCardInfoLabelX = 420;
const int memoryCardInfoValueX = 575;
const int memoryCardInfoLabelWidth = 145;
const int memoryCardInfoValueWidth = 300;
const int memoryCardInfoStartY = 130;
const int memoryCardInfoLineHeight = 27;
const int memoryCardInfoMaxFontSize = 18;
const int memoryCardInfoMinFontSize = 12;
const int memoryCardTitleMaxFontSize = 20;
const int memoryCardSummaryMaxFontSize = 18;
const int memoryCardSummaryY = 500;
const int memoryCardSummaryWidth = 560;
const int memoryCardJisFontSize = 20;
const int memoryCardPencilSize = 42;
const int memoryCardGridLeftX = 80;
const int memoryCardGridRightX = 940;
const int memoryCardGridY = 80;
const int memoryCardGridIconInset = 10;
const int memoryCardSlotIconSize = 75;
const int memoryCardGridCellSize = 80;
const int memoryCardGridColumns = 3;
const int memoryCardGridRows = 5;
const int memoryCardTitleToDetailsGap = 4;
const int memoryCardAnimationFrameDelay = 5;
const SDL_Color memoryCardInfoLabelColor = {80, 170, 255, 255};

int countUsedSlots(CardEdit *card) {
    int usedSlots = 0;
    for (int slot = 0; slot < memoryCardSlotCount; slot++) {
        if (card->get_slot_is_used(slot)) {
            usedSlots++;
        }
    }

    return usedSlots;
}

int countFreeSlots(CardEdit *card) {
    int freeSlots = 0;
    for (int slot = 0; slot < memoryCardSlotCount; slot++) {
        if (card->get_slot_is_free(slot)) {
            freeSlots++;
        }
    }

    return freeSlots;
}

int findOwnerSlot(CardEdit *card, int selectedSlot) {
    if (card->get_slot_is_free(selectedSlot)) {
        return -1;
    }
    if (card->is_slot_top(selectedSlot)) {
        return selectedSlot;
    }

    for (int slot = 0; slot < memoryCardSlotCount; slot++) {
        if (!card->is_slot_top(slot) || card->get_slot_is_free(slot)) {
            continue;
        }
        vector<int> gameSlots = card->getGameSlots(slot);
        for (int gameSlot : gameSlots) {
            if (gameSlot == selectedSlot) {
                return slot;
            }
        }
    }

    return -1;
}

string dashIfEmpty(const string &text) { return text.empty() ? "-" : text; }

string formatSlotNumber(int slot) { return to_string(slot + 1); }

string formatNextSlot(CardEdit *card, int slot) {
    int nextSlot = card->next_slot_map[slot];
    if (nextSlot < 0 || nextSlot >= memoryCardSlotCount) {
        return "-";
    }

    return formatSlotNumber(nextSlot);
}

string formatSlotStatus(CardEdit *card, int slot) {
    if (card->get_slot_is_free(slot)) {
        return _("Empty");
    }
    if (card->is_slot_top(slot)) {
        return _("Save start");
    }

    return _("Linked block");
}

string formatBlockInfo(CardEdit *card, int selectedSlot) {
    int ownerSlot = findOwnerSlot(card, selectedSlot);
    if (ownerSlot < 0) {
        return "-";
    }

    string blocks = to_string(card->getGameSlots(ownerSlot).size());
    if (ownerSlot == selectedSlot) {
        return blocks;
    }

    return blocks + " (" + _("starts at slot") + " " + formatSlotNumber(ownerSlot) + ")";
}

string formatCardSummary(const string &cardName, CardEdit *card) {
    return cardName + "  " + to_string(countUsedSlots(card)) + "/" + to_string(memoryCardSlotCount) + " " + _("Slot") +
           "  " + _("Free") + ": " + to_string(countFreeSlots(card));
}

bool copyGameToCard(CardEdit *src, CardEdit *dest, int slot) {
    int gameSize = src->getGameSlots(slot).size();
    if (dest->findEmptySlot(gameSize).empty()) {
        return false;
    }

    int exportSize = src->getExportSize(slot);
    if (exportSize <= 0) {
        return false;
    }

    vector<unsigned char> buffer(exportSize);
    src->exportGame(slot, buffer.data());
    dest->importGame(buffer.data(), exportSize);
    return true;
}

void renderInfoLine(shared_ptr<Gui> gui, const string &label, const string &value, int y, FC_Font_Shared valueFont) {
    gui->renderFittedText_WithColor(gui->themeFonts[FONT_20_BOLD], label, memoryCardInfoLabelX, y,
                                    memoryCardInfoLabelWidth, memoryCardInfoMaxFontSize, memoryCardInfoMinFontSize,
                                    memoryCardInfoLabelColor);
    gui->renderFittedText(valueFont, value, memoryCardInfoValueX, y, memoryCardInfoValueWidth,
                          memoryCardInfoMaxFontSize, memoryCardInfoMinFontSize);
}
} // namespace

void GuiMcManager::init() {
    rightCardName_ori = rightCardName;
    cardPath_ori = card2path;
    loadAssets();
}

void GuiMcManager::loadAssets() {
    shared_ptr<Gui> gui(Gui::getInstance());
    mcGrid = IMG_LoadTexture(renderer, (gui->getCurrentThemeImagePath() + sep + "MC/Dot_Matrix.png").c_str());
    mcPencil = IMG_LoadTexture(renderer, (gui->getCurrentThemeImagePath() + sep + "MC/Pencil_Carsor.png").c_str());
    fontJIS = Fonts::openNewSharedCachedFont(Fonts::getResourceFontPath(Env::getWorkingPath(), "japanese.ttf"),
                                             memoryCardJisFontSize, renderer);

    memcard1 = new CardEdit(renderer);
    memcard2 = new CardEdit(renderer);

    memcard1->load_file(card1path);
    memcard2->load_file(card2path);

    pencilPos.w = memoryCardPencilSize;
    pencilPos.h = memoryCardPencilSize;
    pencilPos.x = mc1XStart;
    pencilPos.y = mcYStart;
    pencilColumn = 0;
    pencilRow = 0;
    pencilMemcard = 1;
}

void GuiMcManager::pencilDown() {
    if (pencilRow != memoryCardGridRows - 1) {
        pencilRow++;
    }
}

void GuiMcManager::pencilUp() {
    if (pencilRow != 0) {
        pencilRow--;
    }
}

void GuiMcManager::pencilLeft() {
    if (pencilColumn != 0) {
        pencilColumn--;
    } else {
        pencilColumn = 2;
        if (pencilMemcard == 1)
            pencilMemcard = 2;
        else
            pencilMemcard = 1;
    }
}

void GuiMcManager::pencilRight() {
    if (pencilColumn != 2) {
        pencilColumn++;
    } else {
        pencilColumn = 0;
        if (pencilMemcard == 1)
            pencilMemcard = 2;
        else
            pencilMemcard = 1;
    }
}

void GuiMcManager::renderPencil(int memcard, int col, int row) {
    if (memcard == 1) {
        pencilPos.x = mc1XStart + (col * memoryCardGridCellSize);
    }
    if (memcard == 2) {
        pencilPos.x = mc2XStart + (col * memoryCardGridCellSize);
    }
    pencilPos.y = mcYStart + (row * memoryCardGridCellSize);
    SDL_RenderCopy(renderer, mcPencil, nullptr, &pencilPos);
}

void GuiMcManager::trySave() {
    if (changes) {
        auto confirm = new GuiConfirm(renderer);
        confirm->label = _("Do you want to save memcards data?");
        confirm->show();
        if (confirm->result) {
            memcard1->save_file(card1path);
            memcard2->save_file(card2path);
            changes = false;
        }
        delete (confirm);
        changes = false;
    }
}

void GuiMcManager::renderStatic() {
    shared_ptr<Gui> gui(Gui::getInstance());
    gui->renderBackground();
    gui->renderTextBar();
    gui->renderTitleLine(_("Memory Card Manager"), 0, gui->getContentTopY());
    gui->renderStatus("|@Start| " + _("Select Right Card") + " | |@Select| " + _("Defragment Card") + "   | " +
                      "|@X| " + _("Reload Cards") + "   | " + "|@T| " + _("Delete") + " | " + "|@S| " + _("Copy") +
                      " | " + "|@O| " + _("Go back") + "|");

    // Draw dot matrix image
    SDL_Rect input, output;
    SDL_QueryTexture(mcGrid, nullptr, nullptr, &input.w, &input.h);
    SDL_QueryTexture(mcGrid, nullptr, nullptr, &output.w, &output.h);
    input.x = 0, input.y = 0;
    output.x = memoryCardGridLeftX;
    output.y = memoryCardGridY;
    SDL_RenderCopy(renderer, mcGrid, &input, &output);
    output.x = memoryCardGridRightX;
    output.y = memoryCardGridY;
    SDL_RenderCopy(renderer, mcGrid, &input, &output);
}

void GuiMcManager::renderMemCardIcons(int memcard) {
    SDL_Rect output;
    output.h = memoryCardSlotIconSize;
    output.w = memoryCardSlotIconSize;

    int start;
    CardEdit *currentCard;
    if (memcard == 1) {
        start = memoryCardGridLeftX;
        currentCard = memcard1;
    }

    if (memcard == 2) {
        start = memoryCardGridRightX;
        currentCard = memcard2;
    }

    for (int i = 0; i < memoryCardSlotCount; i++) {
        int col = i % memoryCardGridColumns;
        int line = i / memoryCardGridColumns;
        int frame = 0;
        if ((pencilMemcard == memcard) && (pencilRow == line) && (pencilColumn == col)) {
            frame = animFrame;
        }
        output.x = start + (memoryCardGridCellSize * col) + memoryCardGridIconInset;
        output.y = memoryCardGridY + (memoryCardGridCellSize * line) + memoryCardGridIconInset;
        if (currentCard->get_slot_is_used(i)) {
            SDL_RenderCopy(renderer, currentCard->get_slot_icon(i, frame), nullptr, &output);
        }
    }
}

void GuiMcManager::renderMetaInfo() {
    shared_ptr<Gui> gui(Gui::getInstance());

    CardEdit *card;
    string cardName;
    if (pencilMemcard == 1) {
        card = memcard1;
        cardName = leftCardName;

    } else {
        card = memcard2;
        cardName = rightCardName;
    }

    int slot = pencilColumn + pencilRow * memoryCardGridColumns;
    int y = memoryCardInfoStartY;

    gui->renderFittedText_WithColor(gui->themeFonts[FONT_20_BOLD], _("Selected Slot"), SCREEN_WIDTH / 2, y,
                                    memoryCardInfoWidth, memoryCardTitleMaxFontSize, memoryCardInfoMinFontSize,
                                    Gui::getTitleTextColor(), XALIGN_CENTER);
    y += memoryCardInfoLineHeight + memoryCardTitleToDetailsGap;

    renderInfoLine(gui, _("Card"), cardName, y, gui->themeFonts[FONT_20_BOLD]);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Slot"), formatSlotNumber(slot), y, gui->themeFonts[FONT_20_BOLD]);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Status"), formatSlotStatus(card, slot), y, gui->themeFonts[FONT_20_BOLD]);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Title"), dashIfEmpty(card->get_slot_title(slot)), y, fontJIS);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Game ID"), dashIfEmpty(card->get_slot_gameID(slot)), y, gui->themeFonts[FONT_20_BOLD]);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Product Code"), dashIfEmpty(card->get_slot_Pcode(slot)), y, gui->themeFonts[FONT_20_BOLD]);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Blocks"), formatBlockInfo(card, slot), y, gui->themeFonts[FONT_20_BOLD]);
    y += memoryCardInfoLineHeight;
    renderInfoLine(gui, _("Next Slot"), formatNextSlot(card, slot), y, gui->themeFonts[FONT_20_BOLD]);

    gui->renderFittedTextLine(formatCardSummary(leftCardName, memcard1), -memoryCardSummaryY, 0, XALIGN_LEFT, 0,
                              memoryCardSummaryWidth, memoryCardSummaryMaxFontSize, memoryCardInfoMinFontSize);
    gui->renderFittedTextLine(formatCardSummary(rightCardName, memcard2), -memoryCardSummaryY, 0, XALIGN_RIGHT, 0,
                              memoryCardSummaryWidth, memoryCardSummaryMaxFontSize, memoryCardInfoMinFontSize);
}

void GuiMcManager::render() {
    shared_ptr<Gui> gui(Gui::getInstance());
    // render static elements
    renderStatic();
    // Draw Memcard images and meta info
    renderMemCardIcons(1);
    renderMemCardIcons(2);
    renderMetaInfo();

    // Draw the pencil
    renderPencil(pencilMemcard, pencilColumn, pencilRow);
    SDL_RenderPresent(renderer);
}

void GuiMcManager::loop() {
    shared_ptr<Gui> gui(Gui::getInstance());
    bool menuVisible = true;
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
            case SDL_CONTROLLERBUTTONDOWN:
                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    trySave();
                    menuVisible = false;
                };
                if (e.cbutton.button == SDL_BTN_CROSS) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    trySave();
                    memcard1->load_file(card1path);
                    memcard2->load_file(card2path);
                    changes = false;
                };
                if (e.cbutton.button == SDL_BTN_SELECT) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    CardEdit *newCard = new CardEdit(renderer);
                    CardEdit *src;
                    if (pencilMemcard == 1) {
                        src = memcard1;

                    } else {
                        src = memcard2;
                    }
                    for (int slot = 0; slot < memoryCardSlotCount; slot++) {
                        if (!src->is_slot_top(slot)) {
                            continue;
                        }

                        if (copyGameToCard(src, newCard, slot)) {
                            Mix_PlayChannel(-1, gui->cursor, 0);
                            changes = true;
                        }
                    }
                    if (pencilMemcard == 1) {
                        memcard1 = newCard;
                        delete (src);
                    } else {
                        memcard2 = newCard;
                        delete (src);
                    };
                }

                if (e.cbutton.button == SDL_BTN_START) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    trySave();
                    auto select = new GuiSelectMemcard(renderer);
                    select->listType = MC_MANAGER;
                    select->show();
                    if (select->selected != -1) {
                        if (select->selected == 0) {
                            rightCardName = rightCardName_ori;
                            card2path = cardPath_ori;
                            memcard2->load_file(card2path);
                        } else {
                            // this is custom
                            int cardNumCustom = atoi(select->cardSelected.substr(1, 1).c_str());
                            string memcard = select->cardSelected.substr(4);
                            string cardPath = Env::getPathToMemCardsDir() + sep + memcard + "/card" +
                                              to_string(cardNumCustom) + ".mcd";

                            rightCardName = select->cardSelected;
                            card2path = cardPath;
                            PLOG_DEBUG << "Card:" << cardPath;
                            memcard2->load_file(card2path);
                        }
                        changes = false;
                    }
                    delete select;
                }
                if (e.cbutton.button == SDL_BTN_TRIANGLE) {
                    CardEdit *card;
                    if (pencilMemcard == 1) {
                        card = memcard1;
                    } else {
                        card = memcard2;
                    }
                    int slot = pencilColumn + pencilRow * memoryCardGridColumns;
                    if (!card->is_slot_top(slot)) {
                        Mix_PlayChannel(-1, gui->cancel, 0);
                        continue;
                    }
                    if (card->get_slot_is_free(slot)) {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        continue;
                    }
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    card->delete_game(slot);
                    changes = true;
                };
                if (e.cbutton.button == SDL_BTN_SQUARE) {
                    CardEdit *src, *dest;
                    if (pencilMemcard == 1) {
                        src = memcard1;
                        dest = memcard2;
                    } else {
                        src = memcard2;
                        dest = memcard1;
                    }
                    int slot = pencilColumn + pencilRow * memoryCardGridColumns;
                    if (!src->is_slot_top(slot)) {
                        Mix_PlayChannel(-1, gui->cancel, 0);
                        continue;
                    }
                    if (src->get_slot_is_free(slot)) {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        continue;
                    }

                    if (copyGameToCard(src, dest, slot)) {
                        Mix_PlayChannel(-1, gui->cursor, 0);
                        changes = true;
                    } else {
                        Mix_PlayChannel(-1, gui->cancel, 0);
                    }
                };
                break;

            case SDL_CONTROLLERHATMOTIONDOWN: /* Handle Joystick Motion */
            case SDL_CONTROLLERHATMOTIONUP:
                if (gui->mapper.isCenter(&e)) {
                }
                if (gui->mapper.isLeft(&e)) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    pencilLeft();
                }
                if (gui->mapper.isRight(&e)) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    pencilRight();
                }
                if (gui->mapper.isUp(&e)) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    pencilUp();
                }
                if (gui->mapper.isDown(&e)) {
                    Mix_PlayChannel(-1, gui->cursor, 0);
                    pencilDown();
                }
                break;
            }
        }
        counter++;
        if (counter > memoryCardAnimationFrameDelay) {
            animFrame++;
            if (animFrame > 2)
                animFrame = 0;
            counter = 0;
        }
        render();
    }
}
