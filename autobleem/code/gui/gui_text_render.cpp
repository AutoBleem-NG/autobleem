#include "gui.h"
#include "../log.h"
#include "../utils/string_utils.h"
#include <cassert>
#include <cstring>

using namespace std;

FC_Size Gui::FC_getFontTextSize(FC_Font_Shared font, const char *text) {
    assert(font != nullptr);
    FC_Size size;
    if (font == nullptr || text == nullptr || strlen(text) == 0)
        size.w = 0;
    else
        size.w = FC_GetWidth(font, "%s", text);
    if (font == nullptr)
        size.h = 0;
    else
        size.h = FC_GetLineHeight(font);

    return size;
}

FC_Rect Gui::FC_getFontTextRect(FC_Font_Shared font, const char *text, int x, int y) {
    FC_Size size = FC_getFontTextSize(font, text);
    FC_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = size.w;
    rect.h = size.h;

    return rect;
}

void Gui::AllTextOrEmojiTokenInfo::compute_xy_relativeOffsets() {
    int xOffset = 0;
    for (auto &info : tokenInfos) {
        info.rect.x = xOffset;
        xOffset += info.rect.w;
        info.rect.y = (totalSize.h - info.rect.h) / 2;
        if (info.emoji) {
            info.rect.y += Gui::getEmojiVisualYOffset(info.tokenString);
        }
    }
}

void Gui::AllTextOrEmojiTokenInfo::getTokenInfo(FC_Font_Shared _font, const string &_text) {
    auto gui = Gui::getInstance();
    font = _font;
    if (!font)
        font = gui->themeFont;

    string text = _text;
    if (text.empty())
        text = " ";
    if (text.back() != '|') {
        text = text + "|";
    }
    auto tokenStrings = StringUtils::getTokens(text, '|');

    for (const auto &tokenString : tokenStrings) {
        if (tokenString == "")
            continue;
        TextOrEmojiTokenInfo tokenInfo;
        tokenInfo.tokenString = tokenString;
        if (tokenString[0] == '@') {
            int w, h;
            auto it = gui->buttonTextureMap.find(tokenString.c_str() + 1);
            if (it != gui->buttonTextureMap.end()) {
                tokenInfo.emoji = it->second;
                SDL_QueryTexture(it->second, nullptr, nullptr, &w, &h);
                tokenInfo.rect.x = 0;
                tokenInfo.rect.y = 0;
                tokenInfo.rect.w = w;
                tokenInfo.rect.h = h;
                totalSize.w += w;
                if (h > totalSize.h)
                    totalSize.h = h;
                tokenInfos.emplace_back(tokenInfo);
            } else {
                PLOG_DEBUG << "emoji not found for " << tokenString;
            }
        } else {
            tokenInfo.rect = gui->FC_getFontTextRect(font, tokenString);
            totalSize.w += tokenInfo.rect.w;
            if (tokenInfo.rect.h > totalSize.h)
                totalSize.h = tokenInfo.rect.h;
            tokenInfos.emplace_back(tokenInfo);
        }
    }

    compute_xy_relativeOffsets();
}

void Gui::AllTextOrEmojiTokenInfo::render(int x, int y, XAlignment xAlign) {
    auto gui = Gui::getInstance();
    auto renderer = gui->renderer;

    compute_xy_relativeOffsets();

    if (xAlign != XALIGN_LEFT)
        x = align_xPosition(xAlign, x, totalSize.w);

    if (drawBackgroundRect) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 70);
        SDL_Rect backRect;
        backRect.x = x - 10;
        backRect.y = y - 2;
        backRect.w = totalSize.w + 20;
        backRect.h = totalSize.h + 4;

        SDL_RenderFillRect(renderer, &backRect);
    }

    for (auto &tokenInfo : tokenInfos) {
        if (tokenInfo.emoji) {
            FC_Rect tempRect = tokenInfo.rect;
            tempRect.x += x;
            tempRect.y += y;
            SDL_RenderCopy(renderer, tokenInfo.emoji, nullptr, &tempRect);
        } else {
            if (useTextColor) {
                FC_DrawColor(font, renderer, x + tokenInfo.rect.x, y + tokenInfo.rect.y, textColor, "%s",
                             tokenInfo.tokenString.c_str());
            } else {
                FC_DrawAlign(font, renderer, x + tokenInfo.rect.x, y + tokenInfo.rect.y, FC_ALIGN_LEFT, "%s",
                             tokenInfo.tokenString.c_str());
            }
        }
    }
}

int Gui::renderText(FC_Font_Shared font, const string &text, int x, int y, XAlignment xAlign) {
    AllTextOrEmojiTokenInfo allTokenInfo(font, text);
    allTokenInfo.render(x, y, xAlign);

    return allTokenInfo.totalSize.h;
}

int Gui::renderText_WithColor(FC_Font_Shared font, const std::string &text, int x, int y, SDL_Color textColor,
                              XAlignment xAlign, bool background) {
    AllTextOrEmojiTokenInfo allTokenInfo(font, text);
    allTokenInfo.setTextColor(textColor);
    allTokenInfo.drawBackgroundRect = background;

    allTokenInfo.render(x, y, xAlign);

    return allTokenInfo.totalSize.h;
}

int Gui::renderTextLine(const string &text, int line, int yoffset, XAlignment xAlign, int xoffset,
                        FC_Font_Shared font) {
    if (!font)
        font = themeFont;

    SDL_Rect opscreen = getOpscreenRectOfTheme();
    Uint16 fontHeight = FC_GetLineHeight(font);
    int x = opscreen.x + 10 + xoffset;
    int y = (fontHeight * line) + yoffset;

    if (line < 0) {
        y = -line;
    }

    return renderText(font, text, x, y, xAlign);
}

int Gui::renderTextLineToColumns(const string &textLeft, const string &textRight, int xLeft, int xRight, int line,
                                 int yoffset, FC_Font_Shared font) {
    renderTextLine(textLeft, line, yoffset, XALIGN_LEFT, xLeft, font);
    return renderTextLine(textRight, line, yoffset, XALIGN_LEFT, xRight, font);
}
