#pragma once

#include "gui_font_wrapper.h"
#include <memory>
#include <string>
#include <vector>

class Gui;

namespace GuiActionText {
struct Group {
    std::string controls;
    std::string label;
};

struct Layout {
    std::vector<Group> groups;
    FC_Font_Shared font;
    int groupGap = 0;
    int controlLabelGap = 0;
    int width = 0;
    int height = 0;
};

Layout getLayout(const std::shared_ptr<Gui> &gui, FC_Font_Shared baseFont, const std::string &text, int maxWidth,
                 int maxSize, int minSize);
int getGroupWidth(FC_Font_Shared font, const Group &group, int controlLabelGap);
void renderGroup(FC_Font_Shared font, const Group &group, int x, int y, int controlLabelGap);
void renderLayout(const Layout &layout, int x, int y);
} // namespace GuiActionText
