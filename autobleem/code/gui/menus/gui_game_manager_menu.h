//
// Created by screemer on 2019-01-24.
//
#pragma once

#include "gui_two_column_string_menu.h"
#include "../../lang.h"
#include "../../launcher/ps_game.h"

//********************
// GuiManager
//********************
class GuiManager : public GuiTwoColumnStringMenu {
  public:
    explicit GuiManager(SDL_Shared<SDL_Renderer> _renderer) : GuiTwoColumnStringMenu(_renderer) {}

    void init() override;
    void render() override;
    void renderLineIndexOnRow(int index, int row) override;
    void renderColumnHeaders();
    void renderGameRows();
    void renderTableBottomLine();
    void renderSelectionBox();
    void renderPreview();
    void updatePreviewTextures();
    void syncVisibleWindow();

    std::string getTitle() override;
    std::string getStatusLine() override;

    void doCircle_Pressed() override;
    void doSquare_Pressed() override;
    void doTriangle_Pressed() override;
    void doCross_Pressed() override;

    void doEnter() override { doCross_Pressed(); }
    void doEscape() override { doCircle_Pressed(); }
    void doDelete() override { doSquare_Pressed(); }

    PsGames psGames;
    int previewIndex = -1;
    SDL_Shared<SDL_Texture> previewCover;
    SDL_Shared<SDL_Texture> previewScreenshot;
    static int flushCovers(const char *file, const struct stat *sb, int flag, struct FTW *s);
    static bool sortByTitle(PsGamePtr i, PsGamePtr j) { return SortByCaseInsensitive(i->title, j->title); }
};
