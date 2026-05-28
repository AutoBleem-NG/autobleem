//
// Created by screemer on 2/12/19.
//

#include "ps_meta.h"
#include "ps_game.h"
#include "../utils/string_utils.h"
#include "../utils/time_utils.h"
#include <SDL2/SDL_image.h>
#include "../lang.h"
#include "../engine/ini_file.h"
#include "../utils/file_utils.h"
#include "../environment.h"
#include "../lightgun_games.h"

using namespace std;

namespace {
const int metaRightPadding = 30;
const int metaTitleMaxFontSize = 28;
const int metaTitleMinFontSize = 16;
const int metaDetailMaxFontSize = 15;
const int metaDetailMinFontSize = 11;
const int metaTitleToDetailsOffset = 51;
const int metaDetailLineOffset = 21;
const int metaDetailBlockBottomPadding = 6;
const int metaDetailsToInfoOffset = 6;
const int metaPlayerTextOffsetX = 35;
const int metaDiscIconOffsetX = 135;
const int metaDiscTextOffsetX = 170;
const int metaSystemIconOffsetX = 190;
const int metaIconSpread = 40;
const int metaIconSize = 30;
const int metaIconYOffset = -2;
const char evoIconDir[] = "evoimg/";
const char ps1IconFile[] = "ps1.png";
const char usbIconFile[] = "usb.png";
const char hdIconFile[] = "hd.png";
const char sdIconFile[] = "sd.png";
const char lockIconFile[] = "lock.png";
const char unlockIconFile[] = "unlock.png";
const char cdIconFile[] = "cd.png";
const char favoriteIconFile[] = "favorite.png";
const char retroarchIconFile[] = "ra.png";
const char lightgunIconFile[] = "lightgun.png";
const char lightgun2IconFile[] = "lightgun2.png";

SDL_Shared<SDL_Texture> loadEvoIcon(SDL_Shared<SDL_Renderer> renderer, const string &rootPath, const string &filename) {
    return IMG_LoadTexture(renderer, (rootPath + evoIconDir + filename).c_str());
}

int getCenteredRowY(int rowY, int rowHeight, int itemHeight) { return rowY + ((rowHeight - itemHeight) / 2); }

void renderTextCenteredInRow(FC_Font_Shared font, const string &text, int x, int rowY, int rowHeight) {
    Gui::AllTextOrEmojiTokenInfo textInfo(font, text);
    textInfo.render(x, getCenteredRowY(rowY, rowHeight, textInfo.totalSize.h) + Gui::getTextVisualYOffset(font));
}
} // namespace

//*******************************
// PsMeta::updateTexts
//*******************************
void PsMeta::updateTexts(const string &gameNameTxt, const string &publisherTxt, const string &yearTxt,
                         const string &serial, const string &region, const string &playersTxt, bool internal, bool hd,
                         bool locked, int discs, bool favorite, bool play_using_ra, bool foreign, bool app,
                         const string &last_played, const std::string &_gamePathForLightgunGamesFile,
                         SDL_Color _textColor) {
    this->discs = discs;
    this->internal = internal;
    this->hd = hd;
    this->locked = locked;
    this->favorite = favorite;
    this->play_using_ra = play_using_ra;
    this->gameName = gameNameTxt;
    this->publisher = publisherTxt;
    this->year = yearTxt;
    this->serial = serial;
    this->region = region;
    this->players = playersTxt;
    this->foreign = foreign;
    this->app = app;
    this->last_played = last_played;
    this->gamePathForLightgunGamesFile = _gamePathForLightgunGamesFile;

    textColor = _textColor;
    textColor.a =
        SDL_ALPHA_OPAQUE; // if you're rendering with a different color you need this or it will be transparent

    if (foreign) {
        trim(publisher);
        if (publisher == "DETECT")
            publisher = _("Unknown Core (AutoDetect)");
    }
}

//*******************************
// PsMeta::updateTexts
//*******************************
void PsMeta::updateTexts(PsGamePtr &psGame, SDL_Color _textColor) {
    if (psGame == nullptr)
        return;

    string appendText = psGame->players == 1 ? _("Player") : _("Players");
    if (!psGame->foreign) {
        if (psGame->serial == "") {
            Inifile iniFile;
            iniFile.load(psGame->folder + sep + "Game.ini");
            psGame->serial = iniFile.values["serial"];
            psGame->region = iniFile.values["region"];
        }
        updateTexts(psGame->title, psGame->publisher, to_string(psGame->year), psGame->serial, psGame->region,
                    to_string(psGame->players) + " " + appendText, psGame->internal, psGame->hd, psGame->locked,
                    psGame->cds, psGame->favorite, psGame->play_using_ra, psGame->foreign, psGame->app,
                    TimeUtils::timeToDisplayTimeString(psGame->last_played),
                    psGame->folder, // ps1 game path in /Games
                    _textColor);
    } else {
        if (psGame->app) {
            psGame->serial = "";
            psGame->region = "";

            updateTexts(psGame->title, psGame->publisher, to_string(psGame->year), psGame->serial, psGame->region,
                        to_string(psGame->players) + " " + appendText, psGame->internal, psGame->hd, psGame->locked,
                        psGame->cds, psGame->favorite, psGame->play_using_ra, psGame->foreign, psGame->app,
                        TimeUtils::timeToDisplayTimeString(psGame->last_played), "", _textColor);
        } else {
            psGame->serial = "";
            psGame->region = "";

            updateTexts(psGame->title, psGame->core_name, to_string(psGame->year), psGame->serial, psGame->region,
                        to_string(psGame->players) + " " + appendText, psGame->internal, psGame->hd, psGame->locked,
                        psGame->cds, psGame->favorite, psGame->play_using_ra, psGame->foreign, psGame->app,
                        TimeUtils::timeToDisplayTimeString(psGame->last_played),
                        psGame->image_path, // Retroarch roms image path
                        _textColor);
        }
    }
}

//*******************************
// PsMeta::destroy
//*******************************
void PsMeta::destroy() {}

//*******************************
// PsMeta::render
//*******************************
void PsMeta::render() {
    if (gameName == "") {
        return;
    }

    if (internalOffTex == nullptr) {
        string curPath = Env::getWorkingPath() + sep;
        internalOnTex = loadEvoIcon(renderer, curPath, ps1IconFile);
        internalOffTex = loadEvoIcon(renderer, curPath, usbIconFile);
        hdOnTex = loadEvoIcon(renderer, curPath, hdIconFile);
        hdOffTex = loadEvoIcon(renderer, curPath, sdIconFile);
        lockOnTex = loadEvoIcon(renderer, curPath, lockIconFile);
        lockOffTex = loadEvoIcon(renderer, curPath, unlockIconFile);
        cdTex = loadEvoIcon(renderer, curPath, cdIconFile);
        favoriteTex = loadEvoIcon(renderer, curPath, favoriteIconFile);
        raTex = loadEvoIcon(renderer, curPath, retroarchIconFile);
        lightgunTex = loadEvoIcon(renderer, curPath, lightgunIconFile);
        lightgun2Tex = loadEvoIcon(renderer, curPath, lightgun2IconFile);
    }

    if (visible) {
        Uint32 format;
        int access;
        int w, h;
        SDL_Rect rect;
        SDL_Rect fullRect;

        auto nameFont = fonts[FONT_28_BOLD];
        auto otherFont = fonts[FONT_15_BOLD];
        const int textMaxWidth = SCREEN_WIDTH - x - metaRightPadding;

        int yOffset = 0;
        //
        // game name line
        //
        gui->renderFittedText(nameFont, gameName, x, y + yOffset, textMaxWidth, metaTitleMaxFontSize,
                              metaTitleMinFontSize);

        //
        // publisher line
        //
        yOffset += metaTitleToDetailsOffset;
        string publisherLine = !foreign ? publisher + ", " + year : publisher;
        gui->renderFittedText(otherFont, publisherLine, x, y + yOffset, textMaxWidth, metaDetailMaxFontSize,
                              metaDetailMinFontSize);

        //
        // if PS1
        //
        if (!foreign) {
            //
            // serial number line
            //
            yOffset += metaDetailLineOffset;
            gui->renderFittedText(otherFont, _("Serial") + ": " + serial + ", " + _("Region") + ": " + region, x,
                                  y + yOffset, textMaxWidth, metaDetailMaxFontSize, metaDetailMinFontSize);

            //
            // last played line
            //
            yOffset += metaDetailLineOffset + metaDetailBlockBottomPadding;
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
            // the devel system has time
            gui->renderFittedText(otherFont, _("Last Played") + ": " + last_played, x, y + yOffset, textMaxWidth,
                                  metaDetailMaxFontSize, metaDetailMinFontSize);
#else
            if (Env::autobleemKernel)
                gui->renderFittedText(otherFont, _("Last Played") + ": " + last_played, x, y + yOffset, textMaxWidth,
                                      metaDetailMaxFontSize, metaDetailMinFontSize);
#endif
            yOffset += metaDetailsToInfoOffset;
        }

        int spreadCount = 1;

        //
        // if PS1
        //
        if (!foreign) {
            const int infoRowY = y + yOffset + metaIconYOffset;
            const int infoRowHeight = metaIconSize;

            //
            // num players
            //
            renderTextCenteredInRow(otherFont, players, x + metaPlayerTextOffsetX, infoRowY, infoRowHeight);

            SDL_QueryTexture(tex, &format, &access, &w, &h);
            rect.x = x;
            rect.y = getCenteredRowY(infoRowY, infoRowHeight, h);
            rect.w = w;
            rect.h = h;

            fullRect.x = 0;
            fullRect.y = 0;
            fullRect.w = w;
            fullRect.h = h;
            SDL_RenderCopy(renderer, tex, &fullRect, &rect);

            //
            // render internal icon
            //
            rect.x = x + metaDiscIconOffsetX;
            SDL_RenderCopy(renderer, cdTex, &fullRect, &rect);

            renderTextCenteredInRow(otherFont, to_string(discs), x + metaDiscTextOffsetX, infoRowY, infoRowHeight);

            rect.x = x + metaSystemIconOffsetX;
            rect.y = getCenteredRowY(infoRowY, infoRowHeight, metaIconSize);
            rect.w = metaIconSize;
            rect.h = metaIconSize;

            fullRect.x = 0;
            fullRect.y = 0;
            fullRect.w = metaIconSize;
            fullRect.h = metaIconSize;
            if (internal) {
                locked = true;
                hd = false;
                SDL_RenderCopy(renderer, internalOnTex, &fullRect, &rect);
            } else {
                SDL_RenderCopy(renderer, internalOffTex, &fullRect, &rect);
            }

            //
            // HD icon
            //
            rect.x = x + metaSystemIconOffsetX + (metaIconSpread * spreadCount);
            if (hd) {
                SDL_RenderCopy(renderer, hdOnTex, &fullRect, &rect);
            } else {
                SDL_RenderCopy(renderer, hdOffTex, &fullRect, &rect);
            }

            //
            // lock icon
            //
            ++spreadCount;
            rect.x = x + metaSystemIconOffsetX + (metaIconSpread * spreadCount);
            if (locked) {
                SDL_RenderCopy(renderer, lockOnTex, &fullRect, &rect);
            } else {
                SDL_RenderCopy(renderer, lockOffTex, &fullRect, &rect);
            }

            //
            // favorite icon
            //
            if (favorite) {
                ++spreadCount;
                rect.x = x + metaSystemIconOffsetX + (metaIconSpread * spreadCount);
                SDL_RenderCopy(renderer, favoriteTex, &fullRect, &rect);
            }

            //
            // play using RA icon
            //
            if (play_using_ra) {
                ++spreadCount;
                rect.x = x + metaSystemIconOffsetX + (metaIconSpread * spreadCount);
                SDL_RenderCopy(renderer, raTex, &fullRect, &rect);
            }

            //
            // light gun icon
            //
            if (Gui::getInstance()->lightgunGames.IsGameALightgunGame(gamePathForLightgunGamesFile)) {
                ++spreadCount;
                rect.x = x + metaSystemIconOffsetX + (metaIconSpread * spreadCount);
                if (players.size() > 0 && players[0] >= '2')
                    SDL_RenderCopy(renderer, lightgun2Tex, &fullRect, &rect);
                else
                    SDL_RenderCopy(renderer, lightgunTex, &fullRect, &rect);
            }
        } else {
            //
            // if RA game
            //
            if (!app) {
                //
                // retroarch icon
                //
                yOffset += metaDetailLineOffset;
                SDL_QueryTexture(raTex, &format, &access, &w, &h);
                rect.x = x;
                rect.y = y + yOffset + metaIconYOffset;
                rect.w = w;
                rect.h = h;

                fullRect.x = 0;
                fullRect.y = 0;
                fullRect.w = w;
                fullRect.h = h;
                SDL_RenderCopy(renderer, raTex, &fullRect, &rect);

                //
                // favorite icon
                //
                if (favorite) {
                    rect.x += metaIconSpread;
                    SDL_RenderCopy(renderer, favoriteTex, &fullRect, &rect);
                }

                //
                // light gun icon
                //
                if (Gui::getInstance()->lightgunGames.IsGameALightgunGame(gamePathForLightgunGamesFile)) {
                    rect.x += metaIconSpread;
                    SDL_RenderCopy(renderer, lightgunTex, &fullRect, &rect);
                }
            }
        }
    }
}

//*******************************
// PsMeta::update
//*******************************
void PsMeta::update(long time) {
    if (visible)
        if (animEndTime != 0) {
            if (animStarted == 0) {
                animStarted = time;
            }

            if (animStarted != 0) {
                // calculate length for point in time
                long currentAnim = time - animStarted;
                long totalAnimTime = animEndTime - animStarted;
                float position = currentAnim * 1.0f / totalAnimTime * 1.0f;
                int newPos = prevPos + ((nextPos - prevPos) * position);
                y = newPos;
            }

            if (time >= animEndTime) {
                animStarted = 0;
                animEndTime = 0;
                y = nextPos;
            }
        }
    lastTime = time;
}
