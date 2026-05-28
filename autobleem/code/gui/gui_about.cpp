//
// Created by screemer on 2019-01-24.
//

#include "gui_about.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include "gui.h"
#include "gui_layout.h"
#include "gui_text.h"
#include "../lang.h"
#include "../engine/scanner.h"
#include "../environment.h"
#include "version.h"

using namespace std;

static const SDL_Color aboutSectionTitleColor = {80, 170, 255, 255};
const int aboutSectionHeadingMaxFontSize = 23;
const int aboutSectionHeadingMinFontSize = 16;
const int aboutSectionBodyMaxFontSize = 20;
const int aboutSectionBodyMinFontSize = 14;
const int aboutTitleMaxFontSize = 39;
const int aboutTitleMinFontSize = 26;
const int aboutAuthorMaxFontSize = 23;
const int aboutAuthorMinFontSize = 16;
const int aboutSupportMaxFontSize = 20;
const int aboutSupportMinFontSize = 14;
const int aboutFooterMaxFontSize = 18;
const int aboutFooterMinFontSize = 13;

static string getDisplayVersion() {
    string version = Version::VERSION;
    if (!version.empty() && version[0] != 'v' && version[0] != 'V') {
        version = "v" + version;
    }

    return version;
}

void GuiAbout::renderCreditSection(const string &heading, const vector<string> &lines, const SDL_Rect &rect) {
    GuiLayout::renderPanel(renderer, rect, {0, 0, 0, 125}, {255, 255, 255, 45});

    SDL_Rect headingRect = GuiLayout::insetRect(rect, 18, 12);
    headingRect.h = 32;
    FC_Font_Shared fittedHeadingFont =
        GuiText::getFittingFont(renderer, fontCache, headingFontPath, aboutSectionHeadingMaxFontSize,
                                aboutSectionHeadingMinFontSize, heading, headingRect.w, headingRect.h);
    GuiText::renderWrappedText(renderer, fittedHeadingFont, heading, headingRect, aboutSectionTitleColor);

    string bodyText = GuiText::joinLines(lines, "\n");
    SDL_Rect bodyRect = GuiLayout::insetRect(rect, 18, 10);
    bodyRect.y = rect.y + 50;
    bodyRect.h = rect.h - 60;
    FC_Font_Shared fittedBodyFont =
        GuiText::getFittingFont(renderer, fontCache, bodyFontPath, aboutSectionBodyMaxFontSize,
                                aboutSectionBodyMinFontSize, bodyText, bodyRect.w, bodyRect.h);
    GuiText::renderWrappedText(renderer, fittedBodyFont, bodyText, bodyRect, {220, 220, 220, 255});
}

void GuiAbout::init() {
    std::shared_ptr<Gui> gui(Gui::getInstance());
    fx.renderer = renderer;
    fontCache.clear();

    string themePath = gui->getCurrentThemePath() + sep;
    bodyFontPath = GuiText::resolveSelectedBodyFontPath(gui);
    titleFontPath = GuiText::resolveThemeFontPath(themePath, "about.ttf");
    headingFontPath = bodyFontPath;
    if (Fonts::currentLanguageNeedsCjkFont()) {
        titleFontPath = bodyFontPath;
        headingFontPath = bodyFontPath;
    } else if (gui->cfg.inifile.get("themefont") == "true") {
        titleFontPath = bodyFontPath;
    }
    logo = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "ablogo.png").c_str());
}

//*******************************
// GuiAbout::render
//*******************************
void GuiAbout::render() {
    std::shared_ptr<Gui> gui(Gui::getInstance());
    string titleText = "AutoBleem-NG " + getDisplayVersion();
    string versionLine = string("Version: ") + Version::VERSION + " (" + Version::GIT_BRANCH + "@" + Version::GIT_HASH +
                         Version::GIT_CHANGED_FLAG + ")";
    string buildTimeLine = string("Built: ") + Version::BUILD_TIMESTAMP + " UTC";

    gui->renderBackground();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 218);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect rect2;
    rect2.x = 0;
    rect2.y = 0;
    rect2.w = SCREEN_WIDTH;
    rect2.h = SCREEN_HEIGHT;

    SDL_RenderFillRect(renderer, &rect2);

    fx.render();

    SDL_Rect header{90, 38, 1100, 130};
    GuiLayout::renderPanel(renderer, header, {0, 0, 0, 95}, {255, 255, 255, 50});

    SDL_Rect logoRect;
    logoRect.x = header.x + 30;
    logoRect.y = header.y + 16;
    logoRect.w = 138;
    logoRect.h = 98;
    SDL_RenderCopy(renderer, logo, nullptr, &logoRect);

    int textInset = 196;
    SDL_Rect titleRect{header.x + textInset, header.y + 16, header.w - (textInset * 2), 46};
    FC_Font_Shared fittedTitleFont =
        GuiText::getFittingFont(renderer, fontCache, titleFontPath, aboutTitleMaxFontSize, aboutTitleMinFontSize,
                                titleText, titleRect.w, titleRect.h);
    GuiText::renderCenteredWrappedText(renderer, fittedTitleFont, titleText, titleRect, {255, 255, 255, 255});

    string authorLine = "by cornelk";
    SDL_Rect authorRect{header.x + textInset, header.y + 64, header.w - (textInset * 2), 28};
    FC_Font_Shared fittedAuthorFont =
        GuiText::getFittingFont(renderer, fontCache, bodyFontPath, aboutAuthorMaxFontSize, aboutAuthorMinFontSize,
                                authorLine, authorRect.w, authorRect.h);
    GuiText::renderCenteredWrappedText(renderer, fittedAuthorFont, authorLine, authorRect, aboutSectionTitleColor);

    string supportLine = _("Support via Discord") + ": https://discord.gg/AHUS3RM";
    SDL_Rect supportRect{header.x + textInset, header.y + 96, header.w - (textInset * 2), 28};
    FC_Font_Shared fittedSupportFont =
        GuiText::getFittingFont(renderer, fontCache, bodyFontPath, aboutSupportMaxFontSize, aboutSupportMinFontSize,
                                supportLine, supportRect.w, supportRect.h);
    GuiText::renderCenteredWrappedText(renderer, fittedSupportFont, supportLine, supportRect, {180, 220, 255, 255});

    renderCreditSection("Build info", {versionLine, buildTimeLine}, {90, 195, 525, 118});
    renderCreditSection(_("Code C++ and shell scripts"), {"screemer, Axanar, mGGk, nex, genderbent, cornelk"},
                        {665, 195, 525, 118});
    renderCreditSection(_("Graphics"), {"KaonashiFTW, GeekAndy, rubixcube6, NewbornfromHell"}, {90, 340, 525, 118});
    renderCreditSection(_("Testing"), {"MagnusRC, xboxiso, Azazel, Solidius, SupaSAIAN, Kingherb, saptis"},
                        {665, 340, 525, 118});
    renderCreditSection(_("Localization support"),
                        {"nex, Azazel, gadsby, GeekAndy, Pardubak, SupaSAIAN, Mate, Sasha, Jakejj, jolny, StepJefli, "
                         "alucard73, MagnusRC, Quenti"},
                        {90, 485, 525, 112});
    renderCreditSection(_("RetroArch integration"), {"genderbent, libretro contributors"}, {665, 485, 525, 112});

    SDL_Rect footer{90, 620, 1100, 50};
    GuiLayout::renderPanel(renderer, footer, {0, 0, 0, 105}, {255, 255, 255, 40});
    string footerText = _("This is free software. It works AS IS and we take no responsibility for any issues or "
                          "damage");
    SDL_Rect footerTextRect = GuiLayout::insetRect(footer, 18, 8);
    FC_Font_Shared fittedFooterFont =
        GuiText::getFittingFont(renderer, fontCache, bodyFontPath, aboutFooterMaxFontSize, aboutFooterMinFontSize,
                                footerText, footerTextRect.w, footerTextRect.h);
    GuiText::renderCenteredWrappedText(renderer, fittedFooterFont, footerText, footerTextRect, {205, 205, 205, 255});

    gui->renderStatus("|@O| " + _("Go back") + "|", 680);
    SDL_RenderPresent(renderer);
}

//*******************************
// GuiAbout::loop
//*******************************
void GuiAbout::loop() {
    std::shared_ptr<Gui> gui(Gui::getInstance());
    menuVisible = true;
    while (menuVisible) {
        render();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            gui->mapper.handleHotPlug(&e);
            gui->mapper.handlePowerBtn(&e);

            // this is for pc Only
            if (e.type == SDL_QUIT) {
                menuVisible = false;
            }
            switch (e.type) {
            case SDL_CONTROLLERBUTTONDOWN:
                if (e.cbutton.button == SDL_BTN_CIRCLE) {
                    Mix_PlayChannel(-1, gui->cancel, 0);
                    menuVisible = false;
                };
            }
        }
    }
}
