//
// Created by screemer on 2/13/19.
//

#include "ps_carousel.h"
#include "../engine/cover_db.h"
#include "../gui/gui.h"
#include "ra_integrator.h"
#include "thumbnail_lookup.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include "../log.h"
#include "../environment.h"

using namespace std;

#define SLOT_SIZE 120

//*******************************
// PsCarouselGame::loadTex
//*******************************
namespace {

// Resolves the libretro-database canonical name for a game serial, or "" if
// the cover DB isn't loaded / has no record for that serial. The canonical
// name (e.g. "Metal Gear Solid (USA) (Disc 1)") matches the thumbnail file
// name in the libretro-thumbnails pack.
string resolveRecordName(const string &serial) {
    if (serial.empty())
        return "";
    auto gui = Gui::getInstance();
    if (!gui || !gui->coverdb || !gui->coverdb->isValid())
        return "";
    const auto *rec = gui->coverdb->reader.findBySerial(serial);
    return rec ? rec->name : "";
}

} // namespace

void PsCarouselGame::loadTex(SDL_Shared<SDL_Renderer> renderer) {
    shared_ptr<Gui> gui(Gui::getInstance());

    if (!(*this)->foreign) {
        if (coverPng == nullptr) {
            SDL_Shared<SDL_Texture> renderSurface =
                SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, 226, 226);
            SDL_Rect fullRect;

            SDL_SetRenderTarget(renderer, renderSurface);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetTextureBlendMode(renderSurface, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            SDL_RenderFillRect(renderer, nullptr);
            SDL_SetTextureBlendMode(renderSurface, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            const string recordName = resolveRecordName((*this)->serial);

            string imagePath = (*this)->folder + sep + (*this)->base + ".png";
            SDL_SetRenderTarget(renderer, nullptr);
            if (FileUtils::exists(imagePath)) {
                // User-supplied cover dropped next to the game takes priority.
                coverPng = IMG_LoadTexture(renderer, imagePath.c_str());
            } else {
                const string raBoxArt =
                    ThumbnailLookup::findBoxArtPath("Sony - PlayStation", (*this)->title, recordName);
                if (!raBoxArt.empty()) {
                    coverPng = IMG_LoadTexture(renderer, raBoxArt.c_str());
                } else {
                    coverPng = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "default.png").c_str());
                }
            }

            // Screenshot pane for PSX games. The non-foreign branch never
            // populated snapPng before, so the pane sat empty for PS1 titles.
            const string snapPath = ThumbnailLookup::findSnapPath("Sony - PlayStation", (*this)->title, "", recordName);
            snapPng = snapPath.empty() ? nullptr : IMG_LoadTexture(renderer, snapPath.c_str());

            if (coverPng != nullptr) {
                SDL_SetRenderTarget(renderer, renderSurface);
                fullRect.x = 0;
                fullRect.y = 0;
                fullRect.h = 226, fullRect.w = 226;

                Uint32 format;
                int access;
                SDL_QueryTexture(coverPng, &format, &access, &fullRect.w, &fullRect.h);

                SDL_Rect outputRect;
                if (gui->cdJewel != nullptr) {
                    outputRect.x = 23;
                    outputRect.y = 5;
                    outputRect.h = 217;
                    outputRect.w = 199;
                } else {
                    outputRect.x = 0;
                    outputRect.y = 0;
                    outputRect.h = 226;
                    outputRect.w = 226;
                }
                if (coverPng != nullptr) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
                    SDL_RenderCopy(renderer, coverPng, &fullRect, &outputRect);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                }
                coverPng = nullptr;

                fullRect.x = 0;
                fullRect.y = 0;
                fullRect.h = 226, fullRect.w = 226;
                if (gui->cdJewel != nullptr) {
                    SDL_RenderCopy(renderer, gui->cdJewel, &fullRect, &fullRect);
                }
                coverPng = renderSurface;
            }
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        }
    } else { // foreign
        if (coverPng == nullptr) {
            SDL_Shared<SDL_Texture> renderSurface =
                SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, 226, 226);
            SDL_Rect fullRect;

            SDL_SetRenderTarget(renderer, renderSurface);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetTextureBlendMode(renderSurface, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            SDL_RenderFillRect(renderer, nullptr);
            SDL_SetTextureBlendMode(renderSurface, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_SetRenderTarget(renderer, nullptr);
            string imagePath;
            if (!(*this)->app) { // RA Game
                const string recordName = resolveRecordName((*this)->serial);
                imagePath = ThumbnailLookup::findBoxArtPath((*this)->db_name, (*this)->title, recordName);
                if (!imagePath.empty()) {
                    coverPng = IMG_LoadTexture(renderer, imagePath.c_str());
                } else {
                    PLOG_DEBUG << "boxart image NOT found for " << (*this)->title;
                    coverPng = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "evoimg/ra-cover.png").c_str());
                }

                const string snapPath =
                    ThumbnailLookup::findSnapPath((*this)->db_name, (*this)->title, (*this)->image_path, recordName);
                snapPng = snapPath.empty() ? nullptr : IMG_LoadTexture(renderer, snapPath.c_str());
            } else // App
            {
                imagePath = (*this)->image_path;

                if (FileUtils::exists(imagePath)) {
                    coverPng = IMG_LoadTexture(renderer, imagePath.c_str());
                } else {
                    // use default
                    PLOG_DEBUG << "boxart image NOT found for " << imagePath;
                    coverPng =
                        IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + "evoimg/app-cover.png").c_str());
                }
            }

            SDL_Rect imageCoverRect;
            int w, h;

            SDL_SetRenderTarget(renderer, renderSurface);
            fullRect.x = 0;
            fullRect.y = 0;
            fullRect.h = 226, fullRect.w = 226;

            Uint32 format;
            int access;
            SDL_QueryTexture(coverPng, &format, &access, &fullRect.w, &fullRect.h);
            float aspectRatio = (fullRect.w * 1.0f) / (fullRect.h * 1.0f);
            SDL_Rect outputRect;

            // calculate output rect with aspect ratio
            int biggerSize = fullRect.w > fullRect.h ? fullRect.w : fullRect.h;

            outputRect.x = 0;
            outputRect.y = 0;
            outputRect.h = (226 * fullRect.h) / biggerSize;
            outputRect.w = (226 * fullRect.w) / biggerSize;
            outputRect.x = (226 - outputRect.w) / 2;
            outputRect.y = (226 - outputRect.h) / 2;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            SDL_RenderCopy(renderer, coverPng, &fullRect, &outputRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            coverPng = nullptr;
            fullRect.x = 0;
            fullRect.y = 0;
            fullRect.h = 226, fullRect.w = 226;
            coverPng = renderSurface;

            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        }
    }
}

//*******************************
// PsCarouselGame::freeTex
//*******************************
void PsCarouselGame::freeTex() {
    coverPng = nullptr;
    snapPng = nullptr;
}

//*******************************
// PsCarousel::createCoverPoint
//*******************************
PsScreenpoint PsCarousel::createCoverPoint(int x, int shade, int side) {
    shade = 255;
    if (side == 0) {
        PsScreenpoint point;
        point.x = 405 - SLOT_SIZE * x;
        point.y = 100;
        point.scale = 0.5f;
        point.shade = shade;
        return point;
    } else {
        PsScreenpoint point;
        point.x = 405 + 357 + SLOT_SIZE * x;
        point.y = 100;
        point.scale = 0.5f;
        point.shade = shade;
        return point;
    }
}

//*******************************
// PsCarousel::initCoverPositions
//*******************************
void PsCarousel::initCoverPositions() {
    // 405 x 100
    coverPositions.clear();

    coverPositions.push_back(createCoverPoint(5, 40, 0));
    coverPositions.push_back(createCoverPoint(4, 70, 0));
    coverPositions.push_back(createCoverPoint(3, 90, 0));
    coverPositions.push_back(createCoverPoint(2, 100, 0));
    coverPositions.push_back(createCoverPoint(1, 128, 0));
    coverPositions.push_back(createCoverPoint(0, 150, 0));

    PsScreenpoint point;
    point.x = 640 - 113;
    point.y = 180;
    point.scale = 1;
    point.shade = 255;
    coverPositions.push_back(point);

    coverPositions.push_back(createCoverPoint(0, 150, 1));
    coverPositions.push_back(createCoverPoint(1, 128, 1));
    coverPositions.push_back(createCoverPoint(2, 100, 1));
    coverPositions.push_back(createCoverPoint(3, 90, 1));
    coverPositions.push_back(createCoverPoint(4, 70, 1));
    coverPositions.push_back(createCoverPoint(5, 40, 1));

    // special point to move it up
}
