#include <gtest/gtest.h>

#include "../code/environment.h"
#include "../code/gui/gui_layout.h"
#include "../code/gui/gui_text.h"
#include "../code/utils/file_utils.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

class GuiTextTest : public ::testing::Test {
  protected:
    string usbRoot;

    void SetUp() override {
        char tempPath[] = "/tmp/ab_gui_text_XXXXXX";
        ASSERT_NE(nullptr, mkdtemp(tempPath));
        usbRoot = tempPath;

        const char *argv[] = {"gui_text_test", usbRoot.c_str()};
        ASSERT_TRUE(Env::parseCommandLineArguments(2, const_cast<char **>(argv)));
    }

    void TearDown() override {
        Env::resetPaths();
        if (usbRoot != "") {
            FileUtils::removeDirAndContents(usbRoot);
        }
    }

    void writeFile(const string &path) {
        ASSERT_TRUE(FileUtils::ensureParentDirExists(path));
        ofstream out(path);
        ASSERT_TRUE(out.good());
        out << "test";
    }
};

TEST_F(GuiTextTest, JoinLinesSkipsEmptyLinesAndUsesSpacesByDefault) {
    EXPECT_EQ(GuiText::joinLines({"AutoBleem", "", "NG"}), "AutoBleem NG");
}

TEST_F(GuiTextTest, JoinLinesSupportsCustomSeparator) {
    EXPECT_EQ(GuiText::joinLines({"Version", "Built"}, "\n"), "Version\nBuilt");
}

TEST_F(GuiTextTest, InsetRectAppliesHorizontalAndVerticalPadding) {
    SDL_Rect rect{10, 20, 300, 160};

    SDL_Rect inset = GuiLayout::insetRect(rect, 15, 8);

    EXPECT_EQ(inset.x, 25);
    EXPECT_EQ(inset.y, 28);
    EXPECT_EQ(inset.w, 270);
    EXPECT_EQ(inset.h, 144);
}

TEST_F(GuiTextTest, ThemeFontPathFindsFontsSubdirectoryByBaseName) {
    string themePath = usbRoot + "/themes/default/";
    string expectedPath = themePath + "fonts/Oswald.ttf";
    writeFile(expectedPath);

    EXPECT_EQ(Fonts::getThemeFontPath(themePath, "/ignored/path/Oswald.ttf"), expectedPath);
}

TEST_F(GuiTextTest, RetroarchFontPathFindsNamedFont) {
    string expectedPath = usbRoot + "/retroarch/fonts/NotoSansSC.ttf";
    writeFile(expectedPath);

    EXPECT_EQ(Fonts::getRetroarchFontPath("NotoSansSC.ttf"), expectedPath);
}

TEST_F(GuiTextTest, FontPathLookupsRejectEmptyFontName) {
    EXPECT_EQ(Fonts::getRetroarchFontPath(""), "");
    EXPECT_EQ(Fonts::getResourceFontPath(usbRoot, ""), "");
}

TEST_F(GuiTextTest, ResolveThemeFontPathFallsBackToConfiguredResourceFont) {
    string expectedPath = usbRoot + "/Autobleem/bin/autobleem/fonts/Oswald.ttf";
    writeFile(expectedPath);

    EXPECT_EQ(GuiText::resolveThemeFontPath(usbRoot + "/themes/default/", "Oswald.ttf"), expectedPath);
}

TEST_F(GuiTextTest, ResolveThemeFontPathUsesFirstAvailableFontForEmptyFontName) {
    string expectedPath = usbRoot + "/retroarch/fonts/Any.ttf";
    writeFile(expectedPath);

    EXPECT_EQ(GuiText::resolveThemeFontPath(usbRoot + "/themes/default/", ""), expectedPath);
}

TEST_F(GuiTextTest, ResolveFontWithFallbackUsesExplicitFallbackBeforeAnyAvailableFont) {
    string themePath = usbRoot + "/themes/default/";
    string fallbackPath = themePath + "font/Inter.ttf";
    string firstAvailablePath = usbRoot + "/retroarch/fonts/Any.ttf";
    writeFile(fallbackPath);
    writeFile(firstAvailablePath);

    EXPECT_EQ(GuiText::resolveFontWithFallback("Missing.ttf", "Inter.ttf", themePath), fallbackPath);
}

TEST_F(GuiTextTest, ResolveFontWithFallbackUsesFirstRetroarchFontWhenNamedFontsAreMissing) {
    string themePath = usbRoot + "/themes/default/";
    string expectedPath = usbRoot + "/retroarch/fonts/Any.ttf";
    writeFile(expectedPath);

    EXPECT_EQ(GuiText::resolveFontWithFallback("Missing.ttf", "AlsoMissing.ttf", themePath), expectedPath);
}
