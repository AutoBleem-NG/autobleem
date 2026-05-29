//
// Created by screemer on 2/13/19.
//

#include "ps_carousel.h"
#include "../gui/gui.h"
#include "ra_integrator.h"
#include "thumbnail_lookup.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include "../log.h"
#include "../environment.h"

using namespace std;

#define SLOT_SIZE 120

namespace {
const int selectedCoverSize = 226;
const int selectedCoverX = (SCREEN_WIDTH / 2) - (selectedCoverSize / 2);
const int selectedCoverY = 180;
const int jewelCoverX = 23;
const int jewelCoverY = 5;
const int jewelCoverWidth = 199;
const int jewelCoverHeight = 217;
const char localCoverExtension[] = ".png";
const char defaultCoverFile[] = "default.png";
const char retroarchCoverFile[] = "evoimg/ra-cover.png";
const char appCoverFile[] = "evoimg/app-cover.png";

string existingPathOrEmpty(const string &path) { return (!path.empty() && FileUtils::exists(path)) ? path : ""; }

string findPs1RecordName(const PsGame &game, const Coverdb *coverdb) {
    return game.thumbnail_record_name.empty() ? Coverdb::findRecordNameForSerial(coverdb, game.serial)
                                              : game.thumbnail_record_name;
}

string findPs1CoverPath(const PsGame &game, const string &recordName) {
    const string localCoverPath = game.folder + sep + game.base + localCoverExtension;
    if (FileUtils::exists(localCoverPath)) {
        // User-supplied cover dropped next to the game takes priority.
        return localCoverPath;
    }

    const string cachedCoverPath = existingPathOrEmpty(game.cached_cover_path);
    if (!cachedCoverPath.empty()) {
        return cachedCoverPath;
    }

    const string raBoxArt = ThumbnailLookup::findBoxArtPath(ThumbnailLookup::PlayStationDbName, game.title, recordName);
    return raBoxArt.empty() ? Env::getWorkingPath() + sep + defaultCoverFile : raBoxArt;
}

string findPs1SnapPath(const PsGame &game, const string &recordName) {
    const string cachedSnapPath = existingPathOrEmpty(game.cached_snap_path);
    if (!cachedSnapPath.empty()) {
        return cachedSnapPath;
    }
    return ThumbnailLookup::findSnapPath(ThumbnailLookup::PlayStationDbName, game.title, "", recordName);
}
} // namespace

void PsCarouselGame::loadTex(SDL_Shared<SDL_Renderer> renderer) {
    shared_ptr<Gui> gui(Gui::getInstance());
    PsGamePtr game = *this;

    if (!game->foreign) {
        if (coverPng == nullptr) {
            SDL_Shared<SDL_Texture> renderSurface = SDL_CreateTexture(
                renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, selectedCoverSize, selectedCoverSize);
            SDL_Rect fullRect;

            SDL_SetRenderTarget(renderer, renderSurface);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetTextureBlendMode(renderSurface, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            SDL_RenderFillRect(renderer, nullptr);
            SDL_SetTextureBlendMode(renderSurface, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_SetRenderTarget(renderer, nullptr);
            const string recordName = findPs1RecordName(*game, gui->coverdb);
            const string coverPath = findPs1CoverPath(*game, recordName);
            coverPng = IMG_LoadTexture(renderer, coverPath.c_str());

            // Screenshot pane for PSX games. The non-foreign branch never
            // populated snapPng before, so the pane sat empty for PS1 titles.
            const string snapPath = findPs1SnapPath(*game, recordName);
            snapPng = snapPath.empty() ? nullptr : IMG_LoadTexture(renderer, snapPath.c_str());

            if (coverPng != nullptr) {
                SDL_SetRenderTarget(renderer, renderSurface);
                fullRect.x = 0;
                fullRect.y = 0;
                fullRect.h = selectedCoverSize, fullRect.w = selectedCoverSize;

                Uint32 format;
                int access;
                SDL_QueryTexture(coverPng, &format, &access, &fullRect.w, &fullRect.h);

                SDL_Rect outputRect;
                if (gui->cdJewel != nullptr) {
                    outputRect.x = jewelCoverX;
                    outputRect.y = jewelCoverY;
                    outputRect.h = jewelCoverHeight;
                    outputRect.w = jewelCoverWidth;
                } else {
                    outputRect.x = 0;
                    outputRect.y = 0;
                    outputRect.h = selectedCoverSize;
                    outputRect.w = selectedCoverSize;
                }
                if (coverPng != nullptr) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
                    SDL_RenderCopy(renderer, coverPng, &fullRect, &outputRect);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                }
                coverPng = nullptr;

                fullRect.x = 0;
                fullRect.y = 0;
                fullRect.h = selectedCoverSize, fullRect.w = selectedCoverSize;
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
            SDL_Shared<SDL_Texture> renderSurface = SDL_CreateTexture(
                renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, selectedCoverSize, selectedCoverSize);
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
                const string recordName = Coverdb::findRecordNameForSerial(gui->coverdb, (*this)->serial);
                imagePath = ThumbnailLookup::findBoxArtPath((*this)->db_name, (*this)->title, recordName);
                if (!imagePath.empty()) {
                    coverPng = IMG_LoadTexture(renderer, imagePath.c_str());
                } else {
                    PLOG_DEBUG << "boxart image NOT found for " << (*this)->title;
                    coverPng = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + retroarchCoverFile).c_str());
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
                    coverPng = IMG_LoadTexture(renderer, (Env::getWorkingPath() + sep + appCoverFile).c_str());
                }
            }

            SDL_Rect imageCoverRect;
            int w, h;

            SDL_SetRenderTarget(renderer, renderSurface);
            fullRect.x = 0;
            fullRect.y = 0;
            fullRect.h = selectedCoverSize, fullRect.w = selectedCoverSize;

            Uint32 format;
            int access;
            SDL_QueryTexture(coverPng, &format, &access, &fullRect.w, &fullRect.h);
            float aspectRatio = (fullRect.w * 1.0f) / (fullRect.h * 1.0f);
            SDL_Rect outputRect;

            // calculate output rect with aspect ratio
            int biggerSize = fullRect.w > fullRect.h ? fullRect.w : fullRect.h;

            outputRect.x = 0;
            outputRect.y = 0;
            outputRect.h = (selectedCoverSize * fullRect.h) / biggerSize;
            outputRect.w = (selectedCoverSize * fullRect.w) / biggerSize;
            outputRect.x = (selectedCoverSize - outputRect.w) / 2;
            outputRect.y = (selectedCoverSize - outputRect.h) / 2;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            SDL_RenderCopy(renderer, coverPng, &fullRect, &outputRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            coverPng = nullptr;
            fullRect.x = 0;
            fullRect.y = 0;
            fullRect.h = selectedCoverSize, fullRect.w = selectedCoverSize;
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
    point.x = selectedCoverX;
    point.y = selectedCoverY;
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
