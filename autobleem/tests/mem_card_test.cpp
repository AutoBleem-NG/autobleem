// Unit tests for Memcard class
//
// Tests memory card management operations including backup, restore,
// and card repository management. Uses temporary directories for testing.

#include "engine/mem_card.h"
#include "utils/file_utils.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

class MemcardTest : public ::testing::Test {
  protected:
    std::string testDir;
    std::string gamesDir;
    std::string memCardsDir;

    void SetUp() override {
        // Create a unique test directory
        testDir = "/tmp/memcard_test_" + std::to_string(rand());
        gamesDir = testDir + "/Games";
        memCardsDir = gamesDir + "/!MemCards";

        FileUtils::createDir(testDir);
        FileUtils::createDir(gamesDir);
        FileUtils::createDir(memCardsDir);
    }

    void TearDown() override {
        // Clean up test directory
        FileUtils::rmDir(testDir);
    }

    void createTestFile(const std::string &path, const std::string &content = "test") {
        std::ofstream os(path);
        os << content;
        os.close();
    }

    void createGameDir(const std::string &gameName) {
        std::string gamePath = gamesDir + "/" + gameName;
        std::string memcardsPath = gamePath + "/memcards";

        FileUtils::createDir(gamePath);
        FileUtils::createDir(memcardsPath);

        // Create default memory card files
        createTestFile(memcardsPath + "/card1.mcd", "card1_data");
        createTestFile(memcardsPath + "/card2.mcd", "card2_data");
    }

    void createCardRepo(const std::string &cardName) {
        std::string cardPath = memCardsDir + "/" + cardName;
        FileUtils::createDir(cardPath);

        createTestFile(cardPath + "/card1.mcd", cardName + "_card1");
        createTestFile(cardPath + "/card2.mcd", cardName + "_card2");
        createTestFile(cardPath + "/card1.bak", cardName + "_bak1");
        createTestFile(cardPath + "/card2.bak", cardName + "_bak2");
    }

    std::string readFile(const std::string &path) {
        std::ifstream is(path);
        if (!is.is_open()) {
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        is.close();
        return content;
    }
};

//=============================================================================
// Tests for list()
//=============================================================================

TEST_F(MemcardTest, ListReturnsEmptyForNoCards) {
    Memcard mc(gamesDir);
    std::vector<std::string> cards = mc.list();

    EXPECT_TRUE(cards.empty());
}

TEST_F(MemcardTest, ListReturnsSingleCard) {
    createCardRepo("MyCard");

    Memcard mc(gamesDir);
    std::vector<std::string> cards = mc.list();

    ASSERT_EQ(cards.size(), 1);
    EXPECT_EQ(cards[0], "MyCard");
}

TEST_F(MemcardTest, ListReturnsMultipleCards) {
    createCardRepo("Card1");
    createCardRepo("Card2");
    createCardRepo("Card3");

    Memcard mc(gamesDir);
    std::vector<std::string> cards = mc.list();

    EXPECT_EQ(cards.size(), 3);
}

TEST_F(MemcardTest, ListIgnoresFiles) {
    createCardRepo("ValidCard");
    // Create a file (not directory) in the !MemCards folder
    createTestFile(memCardsDir + "/notacard.txt");

    Memcard mc(gamesDir);
    std::vector<std::string> cards = mc.list();

    ASSERT_EQ(cards.size(), 1);
    EXPECT_EQ(cards[0], "ValidCard");
}

//=============================================================================
// Tests for deleteCard()
//=============================================================================

TEST_F(MemcardTest, DeleteCardRemovesDirectory) {
    createCardRepo("ToDelete");
    ASSERT_TRUE(FileUtils::exists(memCardsDir + "/ToDelete"));

    Memcard mc(gamesDir);
    mc.deleteCard("ToDelete");

    EXPECT_FALSE(FileUtils::exists(memCardsDir + "/ToDelete"));
}

TEST_F(MemcardTest, DeleteCardHandlesNonexistent) {
    Memcard mc(gamesDir);
    // Should not throw or crash
    mc.deleteCard("NonexistentCard");
}

//=============================================================================
// Tests for backup()
//=============================================================================

TEST_F(MemcardTest, BackupCreatesBackupDirectory) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    mc.backup(gamePath);

    EXPECT_TRUE(FileUtils::exists(gamePath + "/backup"));
    EXPECT_TRUE(FileUtils::exists(gamePath + "/backup/card1.mcd"));
    EXPECT_TRUE(FileUtils::exists(gamePath + "/backup/card2.mcd"));
}

TEST_F(MemcardTest, BackupPreservesCardContents) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    mc.backup(gamePath);

    EXPECT_EQ(readFile(gamePath + "/backup/card1.mcd"), "card1_data");
    EXPECT_EQ(readFile(gamePath + "/backup/card2.mcd"), "card2_data");
}

TEST_F(MemcardTest, BackupDoesNotOverwriteExisting) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";

    // Create first backup
    Memcard mc(gamesDir);
    mc.backup(gamePath);

    // Modify the original memory cards
    createTestFile(gamePath + "/memcards/card1.mcd", "modified_data");

    // Try to backup again
    mc.backup(gamePath);

    // Backup should still have original data
    EXPECT_EQ(readFile(gamePath + "/backup/card1.mcd"), "card1_data");
}

//=============================================================================
// Tests for restore()
//=============================================================================

TEST_F(MemcardTest, RestoreFromBackup) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    mc.backup(gamePath);

    // Modify the memory cards
    createTestFile(gamePath + "/memcards/card1.mcd", "corrupted");

    // Restore from backup
    mc.restore(gamePath);

    // Verify restored content
    EXPECT_EQ(readFile(gamePath + "/memcards/card1.mcd"), "card1_data");
    // Backup directory should be removed after restore
    EXPECT_FALSE(FileUtils::exists(gamePath + "/backup"));
}

TEST_F(MemcardTest, RestoreHandlesNoBackup) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    // Should not throw when no backup exists
    mc.restore(gamePath);

    // Original memcards should be unchanged
    EXPECT_EQ(readFile(gamePath + "/memcards/card1.mcd"), "card1_data");
}

//=============================================================================
// Tests for swapIn()
//=============================================================================

TEST_F(MemcardTest, SwapInReplacesMemcards) {
    createGameDir("TestGame");
    createCardRepo("SharedCard");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    bool result = mc.swapIn(gamePath, "SharedCard");

    EXPECT_TRUE(result);
    EXPECT_EQ(readFile(gamePath + "/memcards/card1.mcd"), "SharedCard_card1");
    EXPECT_EQ(readFile(gamePath + "/memcards/card2.mcd"), "SharedCard_card2");
}

TEST_F(MemcardTest, SwapInCreatesBackup) {
    createGameDir("TestGame");
    createCardRepo("SharedCard");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    mc.swapIn(gamePath, "SharedCard");

    // Original cards should be backed up
    EXPECT_TRUE(FileUtils::exists(gamePath + "/backup"));
    EXPECT_EQ(readFile(gamePath + "/backup/card1.mcd"), "card1_data");
}

TEST_F(MemcardTest, SwapInReturnsFalseForMissingCard) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    bool result = mc.swapIn(gamePath, "NonexistentCard");

    EXPECT_FALSE(result);
    // Original cards should be unchanged (backup restored)
    EXPECT_EQ(readFile(gamePath + "/memcards/card1.mcd"), "card1_data");
}

//=============================================================================
// Tests for swapOut()
//=============================================================================

TEST_F(MemcardTest, SwapOutCopiesChangesBack) {
    createGameDir("TestGame");
    createCardRepo("SharedCard");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    mc.swapIn(gamePath, "SharedCard");

    // Simulate gameplay - modify the swapped-in cards
    createTestFile(gamePath + "/memcards/card1.mcd", "gameplay_changes");

    mc.swapOut(gamePath, "SharedCard");

    // Changes should be copied back to repository
    EXPECT_EQ(readFile(memCardsDir + "/SharedCard/card1.mcd"), "gameplay_changes");
    // Original cards should be restored
    EXPECT_EQ(readFile(gamePath + "/memcards/card1.mcd"), "card1_data");
}

TEST_F(MemcardTest, SwapOutRestoresBackup) {
    createGameDir("TestGame");
    createCardRepo("SharedCard");
    std::string gamePath = gamesDir + "/TestGame";

    Memcard mc(gamesDir);
    mc.swapIn(gamePath, "SharedCard");
    mc.swapOut(gamePath, "SharedCard");

    // Backup should be removed
    EXPECT_FALSE(FileUtils::exists(gamePath + "/backup"));
}

//=============================================================================
// Tests for storeToRepo()
//=============================================================================

TEST_F(MemcardTest, StoreToRepoCreatesNewCard) {
    createGameDir("TestGame");
    std::string gamePath = gamesDir + "/TestGame";
    std::string memcardsPath = gamePath + "/memcards";

    Memcard mc(gamesDir);
    mc.storeToRepo(memcardsPath, "NewCard");

    EXPECT_TRUE(FileUtils::exists(memCardsDir + "/NewCard"));
    EXPECT_EQ(readFile(memCardsDir + "/NewCard/card1.mcd"), "card1_data");
    EXPECT_EQ(readFile(memCardsDir + "/NewCard/card2.mcd"), "card2_data");
}

TEST_F(MemcardTest, StoreToRepoOverwritesExisting) {
    createGameDir("TestGame");
    createCardRepo("ExistingCard");
    std::string gamePath = gamesDir + "/TestGame";
    std::string memcardsPath = gamePath + "/memcards";

    // Modify game's memcards
    createTestFile(memcardsPath + "/card1.mcd", "updated_data");

    Memcard mc(gamesDir);
    mc.storeToRepo(memcardsPath, "ExistingCard");

    // Repository should have updated data
    EXPECT_EQ(readFile(memCardsDir + "/ExistingCard/card1.mcd"), "updated_data");
}

//=============================================================================
// Tests for rename()
//=============================================================================

TEST_F(MemcardTest, RenameChangesDirectoryName) {
    createCardRepo("OldName");

    Memcard mc(gamesDir);
    mc.rename("OldName", "NewName");

    EXPECT_FALSE(FileUtils::exists(memCardsDir + "/OldName"));
    EXPECT_TRUE(FileUtils::exists(memCardsDir + "/NewName"));
}

TEST_F(MemcardTest, RenamePreservesContents) {
    createCardRepo("OldName");

    Memcard mc(gamesDir);
    mc.rename("OldName", "NewName");

    EXPECT_EQ(readFile(memCardsDir + "/NewName/card1.mcd"), "OldName_card1");
}

TEST_F(MemcardTest, RenameDoesNothingIfNewNameExists) {
    createCardRepo("FirstCard");
    createCardRepo("SecondCard");

    Memcard mc(gamesDir);
    mc.rename("FirstCard", "SecondCard");

    // Both should still exist
    EXPECT_TRUE(FileUtils::exists(memCardsDir + "/FirstCard"));
    EXPECT_TRUE(FileUtils::exists(memCardsDir + "/SecondCard"));
    // SecondCard should have original contents
    EXPECT_EQ(readFile(memCardsDir + "/SecondCard/card1.mcd"), "SecondCard_card1");
}

//=============================================================================
// Tests for restoreAll()
//=============================================================================

TEST_F(MemcardTest, RestoreAllRestoresMultipleGames) {
    createGameDir("Game1");
    createGameDir("Game2");

    Memcard mc(gamesDir);
    mc.backup(gamesDir + "/Game1");
    mc.backup(gamesDir + "/Game2");

    // Modify both games
    createTestFile(gamesDir + "/Game1/memcards/card1.mcd", "modified1");
    createTestFile(gamesDir + "/Game2/memcards/card1.mcd", "modified2");

    mc.restoreAll(gamesDir);

    // Both should be restored
    EXPECT_EQ(readFile(gamesDir + "/Game1/memcards/card1.mcd"), "card1_data");
    EXPECT_EQ(readFile(gamesDir + "/Game2/memcards/card1.mcd"), "card1_data");
}
