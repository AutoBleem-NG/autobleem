#include "gui_layout.h"

#include <algorithm>

namespace GuiLayout {
void renderPanel(SDL_Shared<SDL_Renderer> renderer, const SDL_Rect &rect, SDL_Color fill, SDL_Color border) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &rect);
    renderRectOutline(renderer, rect, border);
}

void renderRectOutline(SDL_Shared<SDL_Renderer> renderer, const SDL_Rect &rect, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

SDL_Rect insetRect(const SDL_Rect &rect, int horizontalInset, int verticalInset) {
    return {rect.x + horizontalInset, rect.y + verticalInset, rect.w - (horizontalInset * 2),
            rect.h - (verticalInset * 2)};
}

SDL_Rect fitRectPreserveAspect(int contentWidth, int contentHeight, const SDL_Rect &bounds) {
    if (contentWidth <= 0 || contentHeight <= 0 || bounds.w <= 0 || bounds.h <= 0) {
        return bounds;
    }

    float scale = std::min(static_cast<float>(bounds.w) / static_cast<float>(contentWidth),
                           static_cast<float>(bounds.h) / static_cast<float>(contentHeight));
    SDL_Rect rect;
    rect.w = static_cast<int>(contentWidth * scale);
    rect.h = static_cast<int>(contentHeight * scale);
    rect.x = bounds.x + ((bounds.w - rect.w) / 2);
    rect.y = bounds.y + ((bounds.h - rect.h) / 2);
    return rect;
}

bool renderTextureFit(SDL_Shared<SDL_Renderer> renderer, SDL_Shared<SDL_Texture> texture, const SDL_Rect &bounds) {
    if (texture == nullptr) {
        return false;
    }

    int textureWidth = 0;
    int textureHeight = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
    if (textureWidth <= 0 || textureHeight <= 0) {
        return false;
    }

    SDL_Rect output = fitRectPreserveAspect(textureWidth, textureHeight, bounds);
    return SDL_RenderCopy(renderer, texture, nullptr, &output) == 0;
}
} // namespace GuiLayout
