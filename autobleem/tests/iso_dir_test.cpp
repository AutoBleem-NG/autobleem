// Unit tests for Isodir class
//
// Tests ISO 9660 directory reading functionality.
// Most functionality requires actual disc images, so tests focus on
// error handling and helper methods.

#include "engine/iso_dir.h"
#include <gtest/gtest.h>
#include <string>

//=============================================================================
// Tests for getEmptyDir()
//=============================================================================

class IsodirEmptyDirTest : public ::testing::Test {};

TEST_F(IsodirEmptyDirTest, ReturnsUnknownValues) {
    Isodir reader;
    IsoDirectory dir = reader.getEmptyDir();

    EXPECT_EQ(dir.systemName, "UNKNOWN");
    EXPECT_EQ(dir.volumeName, "UNKNOWN");
    EXPECT_TRUE(dir.rootDir.empty());
}

TEST_F(IsodirEmptyDirTest, MultipleCallsReturnSameValues) {
    Isodir reader;

    IsoDirectory dir1 = reader.getEmptyDir();
    IsoDirectory dir2 = reader.getEmptyDir();

    EXPECT_EQ(dir1.systemName, dir2.systemName);
    EXPECT_EQ(dir1.volumeName, dir2.volumeName);
}

//=============================================================================
// Tests for getDir() error handling
//=============================================================================

class IsodirGetDirTest : public ::testing::Test {};

TEST_F(IsodirGetDirTest, NonexistentBinReturnsEmptyDir) {
    Isodir reader;
    IsoDirectory dir = reader.getDir("/nonexistent/path/game.bin", 1, false);

    EXPECT_EQ(dir.systemName, "UNKNOWN");
    EXPECT_EQ(dir.volumeName, "UNKNOWN");
    EXPECT_TRUE(dir.rootDir.empty());
}

TEST_F(IsodirGetDirTest, NonexistentChdReturnsEmptyDir) {
    Isodir reader;
    IsoDirectory dir = reader.getDir("/nonexistent/path/game.chd", 1, true);

    EXPECT_EQ(dir.systemName, "UNKNOWN");
    EXPECT_EQ(dir.volumeName, "UNKNOWN");
    EXPECT_TRUE(dir.rootDir.empty());
}

TEST_F(IsodirGetDirTest, EmptyPathReturnsEmptyDir) {
    Isodir reader;
    IsoDirectory dir = reader.getDir("", 1, false);

    EXPECT_EQ(dir.systemName, "UNKNOWN");
    EXPECT_EQ(dir.volumeName, "UNKNOWN");
}

//=============================================================================
// Tests for IsoDirectory struct
//=============================================================================

class IsoDirectoryTest : public ::testing::Test {};

TEST_F(IsoDirectoryTest, DefaultConstruction) {
    IsoDirectory dir;

    // Default constructed should have empty strings and empty vector
    EXPECT_TRUE(dir.systemName.empty());
    EXPECT_TRUE(dir.volumeName.empty());
    EXPECT_TRUE(dir.rootDir.empty());
}

TEST_F(IsoDirectoryTest, CanStoreValues) {
    IsoDirectory dir;
    dir.systemName = "PLAYSTATION";
    dir.volumeName = "GAME_DISC";
    dir.rootDir.push_back("SLUS_001.23");
    dir.rootDir.push_back("SYSTEM.CNF");

    EXPECT_EQ(dir.systemName, "PLAYSTATION");
    EXPECT_EQ(dir.volumeName, "GAME_DISC");
    EXPECT_EQ(dir.rootDir.size(), 2);
    EXPECT_EQ(dir.rootDir[0], "SLUS_001.23");
    EXPECT_EQ(dir.rootDir[1], "SYSTEM.CNF");
}
