//
// Created by screemer on 2019-07-31.
//

#include <random>
#include "starfx.h"
#include "../gui/gui.h"

StarFx::StarFx() {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> wdistribution(0, SCREEN_WIDTH);
    std::uniform_int_distribution<int> hdistribution(0, SCREEN_HEIGHT);

    for (int i = 1; i < 8; i++) {
        for (int star = 0; star < STARS_PER_LAYER; star++) {
            Star s;
            s.speed = i * SPEED_DIFFERENCE;
            s.size = i * SIZE_DIFFERENCE;
            s.x = wdistribution(generator);
            s.y = hdistribution(generator);
            float colorval = 255.0f * (i / 9.0f);
            s.color = RGB(colorval, colorval, colorval);
            s.speed = i * SPEED_DIFFERENCE;
            s.size = i * SIZE_DIFFERENCE;

            starLayers[i - 1].push_back(s);
        }
    }
}

void StarFx::render() {
    for (auto &layer : starLayers) {
        for (auto &star : layer) {
            star.y += star.speed;
            if (star.y > SCREEN_HEIGHT)
                star.y -= SCREEN_HEIGHT;
        }
    }
    for (auto &layer : starLayers) {
        for (auto &star : layer) {
            SDL_Rect rect;
            rect.x = star.x;
            rect.y = star.y;
            rect.w = star.size;
            rect.h = star.size;
            RGB lastColor;
            SDL_GetRenderDrawColor(renderer, &lastColor.r, &lastColor.g, &lastColor.b, &lastColor.a);
            SDL_SetRenderDrawColor(renderer, star.color.r, star.color.g, star.color.b, 255);
            SDL_RenderFillRect(renderer, &rect);
            SDL_GetRenderDrawColor(renderer, &lastColor.r, &lastColor.g, &lastColor.b, &lastColor.a);
        }
    }
}