#pragma once

#include "gui_string_menu.h"
#include <vector>
#include <string>
#include "../../lang.h"

//********************
// GuiMemcards
//********************
class GuiMemcards : public GuiStringMenu {
  public:
    struct CardFileSummary {
        bool exists = false;
        int usedSlots = 0;
        int freeSlots = 0;
        int saves = 0;
        std::vector<std::string> saveTitles;
    };

    struct CustomCardSummary {
        std::string name;
        CardFileSummary card1;
        CardFileSummary card2;
    };

    explicit GuiMemcards(SDL_Shared<SDL_Renderer> _renderer) : GuiStringMenu(_renderer) {}

    void init() override;
    void render() override;
    void renderLineIndexOnRow(int index, int row) override;

    std::string getTitle() override { return _("Custom Memory Cards"); }
    std::string getStatusLine() override; // returns the status line at the bottom

    void doCircle_Pressed() override;
    void doSquare_Pressed() override;
    void doTriangle_Pressed() override;
    void doCross_Pressed() override;

    void doEnter() { doCross_Pressed(); }
    void doEscape() { doCircle_Pressed(); }
    void doDelete() { doSquare_Pressed(); }

  private:
    std::vector<CustomCardSummary> summaries;

    void refreshCards();
    void renderColumnHeaders();
    void renderSelectedCardDetails();
    static CardFileSummary readCardFileSummary(SDL_Shared<SDL_Renderer> renderer, const std::string &path);
};
