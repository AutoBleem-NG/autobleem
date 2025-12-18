// Unit tests for main.h constants and types
#include <gtest/gtest.h>

#include "../code/main.h"

class MainTest : public ::testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ImageType enum tests
TEST_F(MainTest, ImageTypeNoGameFoundIsNegativeOne) {
    EXPECT_EQ(IMAGE_NO_GAME_FOUND, -1);
}

TEST_F(MainTest, ImageTypeBinIsZero) {
    // Must be 0 to match existing game.ini format
    EXPECT_EQ(IMAGE_BIN, 0);
}

TEST_F(MainTest, ImageTypePbpIsOne) {
    // Must be 1 to match existing game.ini format
    EXPECT_EQ(IMAGE_PBP, 1);
}

TEST_F(MainTest, ImageTypeImgIsTwo) {
    EXPECT_EQ(IMAGE_IMG, 2);
}

TEST_F(MainTest, ImageTypeChdIsThree) {
    EXPECT_EQ(IMAGE_CHD, 3);
}

// File extension constants tests
TEST_F(MainTest, ExtensionConstantsStartWithDot) {
    EXPECT_EQ(EXT_PNG[0], '.');
    EXPECT_EQ(EXT_PBP[0], '.');
    EXPECT_EQ(EXT_ECM[0], '.');
    EXPECT_EQ(EXT_BIN[0], '.');
    EXPECT_EQ(EXT_IMG[0], '.');
    EXPECT_EQ(EXT_CHD[0], '.');
    EXPECT_EQ(EXT_CUE[0], '.');
    EXPECT_EQ(EXT_LIC[0], '.');
}

TEST_F(MainTest, ExtensionConstantsAreCorrect) {
    EXPECT_STREQ(EXT_PNG, ".png");
    EXPECT_STREQ(EXT_PBP, ".pbp");
    EXPECT_STREQ(EXT_ECM, ".ecm");
    EXPECT_STREQ(EXT_BIN, ".bin");
    EXPECT_STREQ(EXT_IMG, ".img");
    EXPECT_STREQ(EXT_CHD, ".chd");
    EXPECT_STREQ(EXT_CUE, ".cue");
    EXPECT_STREQ(EXT_LIC, ".lic");
}

// File and directory constants tests
TEST_F(MainTest, GameDataConstantIsCorrect) {
    EXPECT_STREQ(GAME_DATA, "GameData");
}

TEST_F(MainTest, GameIniConstantIsCorrect) {
    EXPECT_STREQ(GAME_INI, "Game.ini");
}

TEST_F(MainTest, PcsxCfgConstantIsCorrect) {
    EXPECT_STREQ(PCSX_CFG, "pcsx.cfg");
}
