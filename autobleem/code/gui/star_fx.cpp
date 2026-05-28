//
// Created by screemer on 2019-07-31.
//

#include <algorithm>
#include <random>
#include <SDL2/SDL_timer.h>
#include "star_fx.h"
#include "../gui/gui.h"

StarFx::StarFx() {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> wdistribution(0, SCREEN_WIDTH);
    std::uniform_int_distribution<int> hdistribution(0, SCREEN_HEIGHT);
    std::uniform_real_distribution<float> driftDistribution(-3.0f, 3.0f);
    lastTicks = SDL_GetTicks();

    for (int i = 1; i < 8; i++) {
        for (int star = 0; star < STARS_PER_LAYER; star++) {
            Star s;
            s.speed = 12.0f + (i * 5.5f * SPEED_DIFFERENCE);
            s.drift = driftDistribution(generator) * (0.35f + (i * 0.08f));
            s.size = i * SIZE_DIFFERENCE;
            s.x = wdistribution(generator);
            s.y = hdistribution(generator);
            unsigned char colorval = static_cast<unsigned char>(180 + (i * 8));
            unsigned char alpha = static_cast<unsigned char>(70 + (i * 16));
            s.color = RGB(colorval, colorval, colorval, alpha);

            starLayers[i - 1].push_back(s);
        }
    }
}

void StarFx::render() {
    Uint32 ticks = SDL_GetTicks();
    float dt = lastTicks == 0 ? 0.0f : (ticks - lastTicks) / 1000.0f;
    lastTicks = ticks;
    dt = std::min(dt, 0.05f);

    for (auto &layer : starLayers) {
        for (auto &star : layer) {
            star.y += star.speed * dt;
            star.x += star.drift * dt;
            if (star.y > SCREEN_HEIGHT)
                star.y -= SCREEN_HEIGHT;
            if (star.x > SCREEN_WIDTH)
                star.x -= SCREEN_WIDTH;
            if (star.x < 0)
                star.x += SCREEN_WIDTH;
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (auto &layer : starLayers) {
        for (auto &star : layer) {
            SDL_Rect rect;
            rect.x = static_cast<int>(star.x);
            rect.y = static_cast<int>(star.y);
            rect.w = std::max(1, static_cast<int>(star.size));
            rect.h = std::max(1, static_cast<int>(star.size));
            RGB lastColor;
            SDL_GetRenderDrawColor(renderer, &lastColor.r, &lastColor.g, &lastColor.b, &lastColor.a);
            SDL_SetRenderDrawColor(renderer, star.color.r, star.color.g, star.color.b, star.color.a);
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, lastColor.r, lastColor.g, lastColor.b, lastColor.a);
        }
    }
}
