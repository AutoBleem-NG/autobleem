// Unit tests for the libretro-thumbnails lookup helpers.
//
// Builds a temporary fake RetroArch tree under /tmp/<unique>/retroarch/ and
// points the Env singleton at it via parseCommandLineArguments — exactly the
// shape findThumbnailPath / findLocalScreenshotPath expect at runtime.

#include "launcher/thumbnail_lookup.h"

#include "environment.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace {

const char *kSystem = "Sony - PlayStation";

void touch(const std::string &path) {
    std::ofstream(path).close();
    ASSERT_EQ(0, ::access(path.c_str(), F_OK));
}

void mkdirs(const std::string &path) {
    std::string acc;
    for (char c : path) {
        if (c == '/') {
            if (!acc.empty())
                ::mkdir(acc.c_str(), 0755);
        }
        acc += c;
    }
    ::mkdir(path.c_str(), 0755);
}

class ThumbnailLookupTest : public ::testing::Test {
  protected:
    std::string usbRoot;
    std::string boxartsDir;
    std::string titlesDir;
    std::string snapsDir;
    std::string screenshotsDir;
    std::string statesDir;

    void SetUp() override {
        char tmpl[] = "/tmp/ab_thumb_XXXXXX";
        ASSERT_NE(nullptr, ::mkdtemp(tmpl));
        usbRoot = tmpl;

        const std::string raDir = usbRoot + "/retroarch";
        boxartsDir = raDir + "/thumbnails/" + kSystem + "/Named_Boxarts";
        titlesDir = raDir + "/thumbnails/" + kSystem + "/Named_Titles";
        snapsDir = raDir + "/thumbnails/" + kSystem + "/Named_Snaps";
        screenshotsDir = raDir + "/screenshots";
        statesDir = raDir + "/states";
        mkdirs(boxartsDir);
        mkdirs(titlesDir);
        mkdirs(snapsDir);
        mkdirs(screenshotsDir);
        mkdirs(statesDir);

        const char *argv[] = {"thumbnail_lookup_test", usbRoot.c_str()};
        ASSERT_TRUE(Env::parseCommandLineArguments(2, const_cast<char **>(argv)));
    }

    void TearDown() override {
        Env::resetPaths();
        std::string cmd = "rm -rf '" + usbRoot + "'";
        std::system(cmd.c_str());
    }
};

} // namespace

TEST_F(ThumbnailLookupTest, EscapeNameReplacesUnsafeChars) {
    EXPECT_EQ("Foo_ Bar", ThumbnailLookup::escapeName("Foo: Bar"));
    EXPECT_EQ("a_b_c_d", ThumbnailLookup::escapeName("a/b/c/d"));
    EXPECT_EQ("Castlevania - Symphony of the Night",
              ThumbnailLookup::escapeName("Castlevania - Symphony of the Night"));
    EXPECT_EQ("__", ThumbnailLookup::escapeName("?|"));
}

TEST_F(ThumbnailLookupTest, FindThumbnailFindsPng) {
    touch(boxartsDir + "/Wild Arms (USA).png");
    EXPECT_EQ(boxartsDir + "/Wild Arms (USA).png",
              ThumbnailLookup::findThumbnailPath(kSystem, "Wild Arms (USA)", "Named_Boxarts"));
}

TEST_F(ThumbnailLookupTest, FindThumbnailFindsJpg) {
    touch(boxartsDir + "/Wild Arms (USA).jpg");
    EXPECT_EQ(boxartsDir + "/Wild Arms (USA).jpg",
              ThumbnailLookup::findThumbnailPath(kSystem, "Wild Arms (USA)", "Named_Boxarts"));
}

TEST_F(ThumbnailLookupTest, FindThumbnailPrefersPngOverJpg) {
    touch(boxartsDir + "/Foo.png");
    touch(boxartsDir + "/Foo.jpg");
    EXPECT_EQ(boxartsDir + "/Foo.png", ThumbnailLookup::findThumbnailPath(kSystem, "Foo", "Named_Boxarts"));
}

TEST_F(ThumbnailLookupTest, FindThumbnailReturnsEmptyForMiss) {
    EXPECT_EQ("", ThumbnailLookup::findThumbnailPath(kSystem, "Nope", "Named_Boxarts"));
}

TEST_F(ThumbnailLookupTest, FindThumbnailRejectsEmptyInputs) {
    touch(boxartsDir + "/X.png");
    EXPECT_EQ("", ThumbnailLookup::findThumbnailPath("", "X", "Named_Boxarts"));
    EXPECT_EQ("", ThumbnailLookup::findThumbnailPath(kSystem, "", "Named_Boxarts"));
    EXPECT_EQ("", ThumbnailLookup::findThumbnailPath(kSystem, "X", ""));
}

TEST_F(ThumbnailLookupTest, FindThumbnailEscapesUnsafeCharsInTitle) {
    touch(boxartsDir + "/Foo_ Bar.png");
    EXPECT_EQ(boxartsDir + "/Foo_ Bar.png",
              ThumbnailLookup::findThumbnailPath(kSystem, "Foo: Bar", "Named_Boxarts"));
}

TEST_F(ThumbnailLookupTest, FindBoxArtPrefersBoxartsOverTitles) {
    touch(boxartsDir + "/Game.png");
    touch(titlesDir + "/Game.png");
    touch(snapsDir + "/Game.png");
    EXPECT_EQ(boxartsDir + "/Game.png", ThumbnailLookup::findBoxArtPath(kSystem, "Game"));
}

TEST_F(ThumbnailLookupTest, FindBoxArtFallsBackToTitles) {
    touch(titlesDir + "/Game.png");
    touch(snapsDir + "/Game.png");
    EXPECT_EQ(titlesDir + "/Game.png", ThumbnailLookup::findBoxArtPath(kSystem, "Game"));
}

TEST_F(ThumbnailLookupTest, FindBoxArtFallsBackToSnaps) {
    touch(snapsDir + "/Game.png");
    EXPECT_EQ(snapsDir + "/Game.png", ThumbnailLookup::findBoxArtPath(kSystem, "Game"));
}

TEST_F(ThumbnailLookupTest, FindLocalScreenshotMatchesPlainName) {
    touch(screenshotsDir + "/Wild Arms (USA).png");
    EXPECT_EQ(screenshotsDir + "/Wild Arms (USA).png",
              ThumbnailLookup::findLocalScreenshotPath("/some/path/Wild Arms (USA).chd"));
}

TEST_F(ThumbnailLookupTest, FindLocalScreenshotPicksNewestTimestamp) {
    touch(screenshotsDir + "/Game-260101-120000.png");
    touch(screenshotsDir + "/Game-260201-080000.png"); // later
    touch(screenshotsDir + "/Game-260101-150000.png");
    EXPECT_EQ(screenshotsDir + "/Game-260201-080000.png",
              ThumbnailLookup::findLocalScreenshotPath("/x/Game.chd"));
}

TEST_F(ThumbnailLookupTest, FindLocalScreenshotIgnoresUnrelatedFiles) {
    touch(screenshotsDir + "/GameTwo.png"); // base "Game" should NOT match "GameTwo"
    EXPECT_EQ("", ThumbnailLookup::findLocalScreenshotPath("/x/Game.chd"));
}

TEST_F(ThumbnailLookupTest, FindLocalScreenshotFallsBackToStateAutoThumb) {
    touch(statesDir + "/Game.state.auto.png");
    EXPECT_EQ(statesDir + "/Game.state.auto.png", ThumbnailLookup::findLocalScreenshotPath("/x/Game.chd"));
}

TEST_F(ThumbnailLookupTest, FindLocalScreenshotPrefersScreenshotsOverState) {
    touch(screenshotsDir + "/Game.png");
    touch(statesDir + "/Game.state.auto.png");
    EXPECT_EQ(screenshotsDir + "/Game.png", ThumbnailLookup::findLocalScreenshotPath("/x/Game.chd"));
}

TEST_F(ThumbnailLookupTest, FindLocalScreenshotEmptyOnNoMatch) {
    EXPECT_EQ("", ThumbnailLookup::findLocalScreenshotPath("/x/Game.chd"));
    EXPECT_EQ("", ThumbnailLookup::findLocalScreenshotPath(""));
}

TEST_F(ThumbnailLookupTest, FindSnapPrefersLocalScreenshotOverNamedSnap) {
    touch(snapsDir + "/Game.png");
    touch(screenshotsDir + "/Game.png");
    EXPECT_EQ(screenshotsDir + "/Game.png",
              ThumbnailLookup::findSnapPath(kSystem, "Game", "/x/Game.chd"));
}

TEST_F(ThumbnailLookupTest, FindSnapFallsBackToNamedSnap) {
    touch(snapsDir + "/Game.png");
    EXPECT_EQ(snapsDir + "/Game.png", ThumbnailLookup::findSnapPath(kSystem, "Game", ""));
}
