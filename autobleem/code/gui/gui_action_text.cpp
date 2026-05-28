#include "gui_action_text.h"
#include "gui.h"
#include "gui_text.h"
#include <algorithm>
#include <cctype>

using namespace std;

namespace GuiActionText {
static bool hasDescription(const string &text) {
    for (unsigned char c : text) {
        if (std::isalnum(c) || c >= 128) {
            return true;
        }
    }

    return false;
}

static bool hasTextOutsideEmojiMarkers(const string &text) {
    string plainText;
    size_t pos = 0;

    while (pos < text.size()) {
        if (text.compare(pos, 2, "|@") == 0) {
            size_t markerEnd = text.find('|', pos + 2);
            if (markerEnd != string::npos) {
                pos = markerEnd + 1;
                continue;
            }
        }

        plainText += text[pos];
        ++pos;
    }

    return hasDescription(plainText);
}

static bool endsWithEmojiMarker(const string &text) {
    size_t markerStart = text.rfind("|@");
    if (markerStart == string::npos || text.empty() || text.back() != '|') {
        return false;
    }

    size_t markerEnd = text.find('|', markerStart + 2);
    return markerEnd == text.size() - 1;
}

static string trimGroup(string text) {
    auto isSpace = [](unsigned char c) { return std::isspace(c); };
    while (!text.empty() && isSpace(text.front())) {
        text.erase(text.begin());
    }
    while (!text.empty() && isSpace(text.back())) {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '|' && !endsWithEmojiMarker(text)) {
        text.pop_back();
    }
    while (!text.empty() && isSpace(text.back())) {
        text.pop_back();
    }

    return text;
}

static string textAfterLastEmojiMarker(const string &text) {
    size_t markerStart = text.rfind("|@");
    if (markerStart == string::npos) {
        return text;
    }

    size_t markerEnd = text.find('|', markerStart + 2);
    if (markerEnd == string::npos || markerEnd + 1 >= text.size()) {
        return "";
    }

    return text.substr(markerEnd + 1);
}

static vector<string> splitGroups(const string &text) {
    vector<string> groups;
    string current;
    size_t pos = 0;

    while (pos < text.size()) {
        if (text.compare(pos, 2, "|@") == 0) {
            size_t markerEnd = text.find('|', pos + 2);
            if (markerEnd == string::npos) {
                current += text.substr(pos);
                break;
            }

            if (!current.empty() && hasDescription(textAfterLastEmojiMarker(current))) {
                string group = trimGroup(current);
                if (!group.empty()) {
                    groups.push_back(group);
                }
                current.clear();
            }

            current += text.substr(pos, markerEnd - pos + 1);
            pos = markerEnd + 1;
            continue;
        }

        size_t nextMarker = text.find("|@", pos);
        if (nextMarker == string::npos) {
            current += text.substr(pos);
            break;
        }
        current += text.substr(pos, nextMarker - pos);
        pos = nextMarker;
    }

    string group = trimGroup(current);
    if (!group.empty()) {
        groups.push_back(group);
    }

    return groups;
}

static Group splitActionGroup(const string &group) {
    size_t markerStart = group.rfind("|@");
    if (markerStart == string::npos) {
        return {"", trimGroup(group)};
    }

    size_t markerEnd = group.find('|', markerStart + 2);
    if (markerEnd == string::npos) {
        return {"", trimGroup(group)};
    }

    string prefix = trimGroup(group.substr(0, markerStart));
    string suffix = trimGroup(group.substr(markerEnd + 1));
    if (hasTextOutsideEmojiMarkers(prefix) && hasTextOutsideEmojiMarkers(suffix)) {
        return {"", trimGroup(group)};
    }

    return {trimGroup(group.substr(0, markerEnd + 1)), trimGroup(group.substr(markerEnd + 1))};
}

static int getGroupHeight(FC_Font_Shared font, const Group &group) {
    int height = 0;
    if (group.controls != "") {
        Gui::AllTextOrEmojiTokenInfo controlInfo(font, group.controls);
        height = std::max(height, controlInfo.totalSize.h);
    }
    if (group.label != "") {
        Gui::AllTextOrEmojiTokenInfo labelInfo(font, group.label);
        height = std::max(height, labelInfo.totalSize.h);
    }

    return height;
}

static int getGapAfterGroup(const Group &leftGroup, const Group &rightGroup, int groupGap, int controlLabelGap) {
    if (leftGroup.controls == "" && leftGroup.label != "" && rightGroup.controls != "") {
        return controlLabelGap;
    }

    return groupGap;
}

static int getGroupsWidth(FC_Font_Shared font, const vector<Group> &groups, int groupGap, int controlLabelGap) {
    int width = 0;
    for (size_t i = 0; i < groups.size(); ++i) {
        width += getGroupWidth(font, groups[i], controlLabelGap);
        if (i + 1 < groups.size()) {
            width += getGapAfterGroup(groups[i], groups[i + 1], groupGap, controlLabelGap);
        }
    }

    return width;
}

static int getGroupsHeight(FC_Font_Shared font, const vector<Group> &groups) {
    int height = FC_GetLineHeight(font);
    for (const Group &group : groups) {
        height = std::max(height, getGroupHeight(font, group));
    }

    return height;
}

int getGroupWidth(FC_Font_Shared font, const Group &group, int controlLabelGap) {
    int width = 0;
    if (group.controls != "") {
        Gui::AllTextOrEmojiTokenInfo controlInfo(font, group.controls);
        width += controlInfo.totalSize.w;
    }
    if (group.label != "") {
        Gui::AllTextOrEmojiTokenInfo labelInfo(font, group.label);
        if (width > 0) {
            width += controlLabelGap;
        }
        width += labelInfo.totalSize.w;
    }

    return width;
}

Layout getLayout(const shared_ptr<Gui> &gui, FC_Font_Shared baseFont, const string &text, int maxWidth, int maxSize,
                 int minSize) {
    if (!baseFont) {
        baseFont = gui->themeFonts[FONT_20_BOLD];
    }

    Layout layout;
    vector<string> rawGroups = splitGroups(text);
    for (const string &group : rawGroups) {
        layout.groups.push_back(splitActionGroup(group));
    }

    FontType fontType = GuiText::getFontTypeForThemeFont(gui.get(), baseFont);
    layout.font = baseFont;
    for (int size = maxSize; size >= minSize; --size) {
        layout.font = size == maxSize ? baseFont : gui->sizesOfThemeFont.GetFont(size, gui->themeFonts, fontType);
        layout.groupGap = max(24, static_cast<int>(FC_GetLineHeight(layout.font)));
        layout.controlLabelGap = max(10, static_cast<int>(FC_GetLineHeight(layout.font) / 2));
        layout.width = getGroupsWidth(layout.font, layout.groups, layout.groupGap, layout.controlLabelGap);
        layout.height = getGroupsHeight(layout.font, layout.groups);
        if (layout.width <= maxWidth) {
            break;
        }
    }

    return layout;
}

void renderGroup(FC_Font_Shared font, const Group &group, int x, int y, int controlLabelGap) {
    int groupHeight = getGroupHeight(font, group);
    if (group.controls != "") {
        Gui::AllTextOrEmojiTokenInfo controlInfo(font, group.controls);
        controlInfo.render(x, y + ((groupHeight - controlInfo.totalSize.h) / 2));
        x += controlInfo.totalSize.w;
        if (group.label != "") {
            x += controlLabelGap;
        }
    }

    if (group.label != "") {
        Gui::AllTextOrEmojiTokenInfo labelInfo(font, group.label);
        labelInfo.render(x, y + ((groupHeight - labelInfo.totalSize.h) / 2) + Gui::getTextVisualYOffset(font));
    }
}

void renderLayout(const Layout &layout, int x, int y) {
    for (size_t i = 0; i < layout.groups.size(); ++i) {
        const Group &group = layout.groups[i];
        int groupHeight = getGroupHeight(layout.font, group);
        renderGroup(layout.font, group, x, y + ((layout.height - groupHeight) / 2), layout.controlLabelGap);
        x += getGroupWidth(layout.font, group, layout.controlLabelGap);
        if (i + 1 < layout.groups.size()) {
            x += getGapAfterGroup(group, layout.groups[i + 1], layout.groupGap, layout.controlLabelGap);
        }
    }
}
} // namespace GuiActionText
