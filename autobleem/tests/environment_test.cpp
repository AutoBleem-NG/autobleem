// Unit tests for environment path configuration
#include <gtest/gtest.h>

#include "../code/environment.h"

class EnvironmentTest : public ::testing::Test {
  protected:
    void SetUp() override { Env::resetPaths(); }
    void TearDown() override { Env::resetPaths(); }
};

TEST_F(EnvironmentTest, SingleArgSetsUSBPath) {
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    int argc = 2;

    bool result = Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    EXPECT_TRUE(result);
    EXPECT_EQ(Env::getPathToUSBRoot(), "/media/usb0");
}

TEST_F(EnvironmentTest, SingleArgSetsGamesPath) {
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    int argc = 2;

    Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    EXPECT_EQ(Env::getPathToGamesDir(), "/media/usb0/Games");
}

TEST_F(EnvironmentTest, SingleArgSetsDatabasePaths) {
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    int argc = 2;

    Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    EXPECT_EQ(Env::getPathToRegionalDBFile(), "/media/usb0/System/Databases/regional.db");
    EXPECT_EQ(Env::getPathToInternalDBFile(), "/media/usb0/System/Databases/internal.db");
}

TEST_F(EnvironmentTest, TwoArgsSetsPathsDirectly) {
    const char *argv[] = {"autobleem-gui", "/path/to/regional.db", "/path/to/Games"};
    int argc = 3;

    bool result = Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    EXPECT_TRUE(result);
    EXPECT_EQ(Env::getPathToRegionalDBFile(), "/path/to/regional.db");
    EXPECT_EQ(Env::getPathToGamesDir(), "/path/to/Games");
}

TEST_F(EnvironmentTest, TwoArgsExtractsUSBPathFromGamesDir) {
    const char *argv[] = {"autobleem-gui", "/path/to/regional.db", "/media/usb0/Games"};
    int argc = 3;

    Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    // USB path is extracted as parent directory of games dir
    EXPECT_EQ(Env::getPathToUSBRoot(), "/media/usb0");
}

TEST_F(EnvironmentTest, NoArgsReturnsFalse) {
    const char *argv[] = {"autobleem-gui"};
    int argc = 1;

    bool result = Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    EXPECT_FALSE(result);
}

TEST_F(EnvironmentTest, TooManyArgsReturnsFalse) {
    const char *argv[] = {"autobleem-gui", "/path1", "/path2", "/path3"};
    int argc = 4;

    bool result = Env::parseCommandLineArguments(argc, const_cast<char **>(argv));

    EXPECT_FALSE(result);
}

TEST_F(EnvironmentTest, ResetClearsAllPaths) {
    // First set up some state
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    Env::parseCommandLineArguments(2, const_cast<char **>(argv));
    EXPECT_FALSE(Env::getPathToUSBRoot().empty());

    // Now reset
    Env::resetPaths();

    EXPECT_TRUE(Env::getPathToUSBRoot().empty());
    EXPECT_TRUE(Env::getPathToGamesDir().empty());
    EXPECT_TRUE(Env::getPathToRegionalDBFile().empty());
    EXPECT_TRUE(Env::getPathToInternalDBFile().empty());
}

TEST_F(EnvironmentTest, SingleArgSetsDerivedPaths) {
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    Env::parseCommandLineArguments(2, const_cast<char **>(argv));

    EXPECT_EQ(Env::getPathToAutobleemDir(), "/media/usb0/Autobleem");
    EXPECT_EQ(Env::getPathToAppsDir(), "/media/usb0/Apps");
    EXPECT_EQ(Env::getPathToSystemDir(), "/media/usb0/System");
    EXPECT_EQ(Env::getPathToRetroarchDir(), "/media/usb0/retroarch");
    EXPECT_EQ(Env::getPathToRomsDir(), "/media/usb0/roms");
}

TEST_F(EnvironmentTest, NestedPathsAreCorrect) {
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    Env::parseCommandLineArguments(2, const_cast<char **>(argv));

    EXPECT_EQ(Env::getPathToMemCardsDir(), "/media/usb0/Games/!MemCards");
    EXPECT_EQ(Env::getPathToSaveStatesDir(), "/media/usb0/Games/!SaveStates");
    EXPECT_EQ(Env::getPathToRCDir(), "/media/usb0/Autobleem/rc");
    EXPECT_EQ(Env::getPathToRetroarchPlaylistsDir(), "/media/usb0/retroarch/playlists");
}

TEST_F(EnvironmentTest, RetroarchCoreFilePathIsCorrect) {
    const char *argv[] = {"autobleem-gui", "/media/usb0"};
    Env::parseCommandLineArguments(2, const_cast<char **>(argv));

    EXPECT_EQ(Env::getPathToRetroarchCoreFile(), "/media/usb0/retroarch/cores/km_pcsx_rearmed_neon_libretro.so");
}
