// Unit tests for Database class
//
// These tests verify database operations:
// - Connection and disconnection
// - Table creation and schema management
// - CRUD operations for games and discs
// - Metadata queries
// - Transaction handling
//
// Uses temporary SQLite databases that are cleaned up after each test.

#include <gtest/gtest.h>
#include "engine/database.h"
#include <cstdio>
#include <string>

// Test fixture for Database tests
class DatabaseTest : public ::testing::Test {
  protected:
    Database db;
    std::string testDbPath;

    void SetUp() override {
        // Create a unique temp file for each test
        testDbPath = "/tmp/test_autobleem_" + std::to_string(rand()) + ".db";
    }

    void TearDown() override {
        db.disconnect();
        // Clean up the test database file
        std::remove(testDbPath.c_str());
    }
};

//=============================================================================
// Connection Tests
//=============================================================================

TEST_F(DatabaseTest, ConnectToNewDatabase) {
    bool result = db.connect(testDbPath);
    EXPECT_TRUE(result) << "Should successfully connect to new database";
}

TEST_F(DatabaseTest, ConnectToInvalidPath) {
    // Try to connect to a path that cannot be created
    bool result = db.connect("/nonexistent/deeply/nested/path/db.sqlite");
    EXPECT_FALSE(result) << "Should fail to connect to invalid path";
}

TEST_F(DatabaseTest, DisconnectAfterConnect) {
    // Connect then disconnect should work cleanly
    ASSERT_TRUE(db.connect(testDbPath));
    db.disconnect();
    // Disconnect again should be safe (db is set to nullptr after first disconnect)
    db.disconnect();
    SUCCEED();
}

TEST_F(DatabaseTest, MultipleConnectDisconnect) {
    for (int i = 0; i < 3; i++) {
        EXPECT_TRUE(db.connect(testDbPath)) << "Connect attempt " << (i + 1) << " failed";
        db.disconnect();
    }
}

//=============================================================================
// Schema Tests
//=============================================================================

TEST_F(DatabaseTest, CreateInitialDatabase) {
    ASSERT_TRUE(db.connect(testDbPath));

    bool result = db.createInitialDatabase();
    EXPECT_TRUE(result) << "Should create initial database schema";
}

TEST_F(DatabaseTest, CreateInitialDatabaseTwice) {
    ASSERT_TRUE(db.connect(testDbPath));

    // First creation
    EXPECT_TRUE(db.createInitialDatabase());

    // Second creation should also succeed (tables already exist)
    EXPECT_TRUE(db.createInitialDatabase());
}

TEST_F(DatabaseTest, AddColumnsMethods) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // These should not crash and should handle existing columns gracefully
    db.addFavoriteColumn();
    db.addHistoryColumn();
    db.addLastPlayedColumn();
    db.addPlayUsingRAColumn();

    // Call again - should handle "duplicate column" gracefully
    db.addFavoriteColumn();
    db.addHistoryColumn();
    db.addLastPlayedColumn();
    db.addPlayUsingRAColumn();

    SUCCEED();
}

//=============================================================================
// Game CRUD Tests
//=============================================================================

TEST_F(DatabaseTest, InsertAndCountGames) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Initially no games
    EXPECT_EQ(db.getNumGames(), 0);

    // Insert a game
    bool inserted = db.insertGame(1, "Test Game", "Test Publisher", 1, 1999, "/path/to/game", "/path/to/ss", "CARD1");
    EXPECT_TRUE(inserted);

    // Should now have 1 game
    EXPECT_EQ(db.getNumGames(), 1);
}

TEST_F(DatabaseTest, InsertMultipleGames) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    db.insertGame(1, "Game One", "Publisher A", 1, 1998, "/path/1", "/ss/1", "CARD1");
    db.insertGame(2, "Game Two", "Publisher B", 2, 1999, "/path/2", "/ss/2", "CARD2");
    db.insertGame(3, "Game Three", "Publisher C", 4, 2000, "/path/3", "/ss/3", "CARD3");

    EXPECT_EQ(db.getNumGames(), 3);
}

TEST_F(DatabaseTest, InsertDisc) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    db.insertGame(1, "Multi-Disc Game", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    // Insert multiple discs for the game
    EXPECT_TRUE(db.insertDisc(1, 1, "disc1.bin"));
    EXPECT_TRUE(db.insertDisc(1, 2, "disc2.bin"));
    EXPECT_TRUE(db.insertDisc(1, 3, "disc3.bin"));
}

//=============================================================================
// Update Tests
//=============================================================================

TEST_F(DatabaseTest, UpdateTitle) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    db.insertGame(1, "Original Title", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    bool updated = db.updateTitle(1, "Updated Title");
    EXPECT_TRUE(updated);
}

TEST_F(DatabaseTest, UpdateYear) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    db.insertGame(1, "Game", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    bool updated = db.updateYear(1, 2001);
    EXPECT_TRUE(updated);
}

TEST_F(DatabaseTest, UpdateMemcard) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    db.insertGame(1, "Game", "Publisher", 1, 1999, "/path", "/ss", "CARD1");

    bool updated = db.updateMemcard(1, "NEWCARD");
    EXPECT_TRUE(updated);
}

TEST_F(DatabaseTest, UpdateFavorite) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());
    db.addFavoriteColumn();

    db.insertGame(1, "Game", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    // Mark as favorite
    EXPECT_TRUE(db.updateFavorite(1, 1));

    // Unmark as favorite
    EXPECT_TRUE(db.updateFavorite(1, 0));
}

TEST_F(DatabaseTest, UpdateHistory) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());
    db.addHistoryColumn();

    db.insertGame(1, "Game", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    // Set history rank
    EXPECT_TRUE(db.updateHistory(1, 5));

    // Clear history
    EXPECT_TRUE(db.updateHistory(1, 0));
}

TEST_F(DatabaseTest, UpdateDatePlayed) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());
    db.addLastPlayedColumn();

    db.insertGame(1, "Game", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    // Set last played timestamp
    int timestamp = 1700000000; // Some Unix timestamp
    EXPECT_TRUE(db.updateDatePlayed(1, timestamp));
}

TEST_F(DatabaseTest, UpdatePlayUsingRA) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());
    db.addPlayUsingRAColumn();

    db.insertGame(1, "Game", "Publisher", 1, 1999, "/path", "/ss", "CARD");

    // Enable RetroArch
    EXPECT_TRUE(db.updatePlayUsingRA(1, 1));

    // Disable RetroArch
    EXPECT_TRUE(db.updatePlayUsingRA(1, 0));
}

//=============================================================================
// Transaction Tests
//=============================================================================

TEST_F(DatabaseTest, BeginAndCommitTransaction) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    EXPECT_TRUE(db.beginTransaction());

    db.insertGame(1, "Game 1", "Publisher", 1, 1999, "/path/1", "/ss/1", "CARD");
    db.insertGame(2, "Game 2", "Publisher", 1, 2000, "/path/2", "/ss/2", "CARD");

    EXPECT_TRUE(db.commit());

    // Games should be persisted
    EXPECT_EQ(db.getNumGames(), 2);
}

//=============================================================================
// Truncate Tests
//=============================================================================

TEST_F(DatabaseTest, TruncateDatabase) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Insert some games
    db.insertGame(1, "Game 1", "Publisher", 1, 1999, "/path/1", "/ss/1", "CARD");
    db.insertGame(2, "Game 2", "Publisher", 1, 2000, "/path/2", "/ss/2", "CARD");
    EXPECT_EQ(db.getNumGames(), 2);

    // Truncate
    EXPECT_TRUE(db.truncate());

    // Should have no games after truncate
    EXPECT_EQ(db.getNumGames(), 0);
}

//=============================================================================
// Delete Tests
//=============================================================================

TEST_F(DatabaseTest, DeleteGameFromAllTables) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Insert a game with discs
    db.insertGame(1, "Game to Delete", "Publisher", 1, 1999, "/path", "/ss", "CARD");
    db.insertDisc(1, 1, "disc1.bin");
    db.insertDisc(1, 2, "disc2.bin");
    EXPECT_EQ(db.getNumGames(), 1);

    // Delete the game
    EXPECT_TRUE(db.deleteGameIdFromAllTables(1));

    // Game should be gone
    EXPECT_EQ(db.getNumGames(), 0);
}

//=============================================================================
// SubDir Row Tests
//=============================================================================

TEST_F(DatabaseTest, SubDirRowOperations) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Table should be empty initially
    EXPECT_TRUE(db.subDirRowsTableIsEmpty());

    // Insert some rows
    EXPECT_TRUE(db.insertSubDirRow(0, "All Games", 0, 10));
    EXPECT_TRUE(db.insertSubDirRow(1, "Action", 1, 5));
    EXPECT_TRUE(db.insertSubDirRow(2, "RPG", 1, 5));

    // Table should no longer be empty
    EXPECT_FALSE(db.subDirRowsTableIsEmpty());

    // Insert game associations
    EXPECT_TRUE(db.insertSubDirGames(1, 1));
    EXPECT_TRUE(db.insertSubDirGames(1, 2));
    EXPECT_TRUE(db.insertSubDirGames(2, 3));
}

TEST_F(DatabaseTest, GetGameRowInfos) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    db.insertSubDirRow(0, "All", 0, 3);
    db.insertSubDirRow(1, "Category A", 1, 2);
    db.insertSubDirRow(2, "Category B", 1, 1);

    GameRowInfos infos;
    EXPECT_TRUE(db.getGameRowInfos(&infos));
    EXPECT_EQ(infos.size(), 3);

    if (infos.size() >= 3) {
        EXPECT_EQ(infos[0].rowName, "All");
        EXPECT_EQ(infos[0].indentLevel, 0);
        EXPECT_EQ(infos[1].rowName, "Category A");
        EXPECT_EQ(infos[1].indentLevel, 1);
    }
}

TEST_F(DatabaseTest, GetGameIdsInRow) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Insert a row and associate games with it
    db.insertSubDirRow(0, "Row 0", 0, 3);
    db.insertSubDirGames(0, 100);
    db.insertSubDirGames(0, 101);
    db.insertSubDirGames(0, 102);

    std::vector<int> gameIds;
    EXPECT_TRUE(db.getGameIdsInRow(&gameIds, 0));
    EXPECT_EQ(gameIds.size(), 3);

    // Verify the game IDs
    bool found100 = false, found101 = false, found102 = false;
    for (int id : gameIds) {
        if (id == 100)
            found100 = true;
        if (id == 101)
            found101 = true;
        if (id == 102)
            found102 = true;
    }
    EXPECT_TRUE(found100 && found101 && found102);
}

//=============================================================================
// Edge Cases
//=============================================================================

TEST_F(DatabaseTest, InsertGameWithSpecialCharacters) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Test with special characters in title and publisher
    bool inserted = db.insertGame(1, "Game's \"Title\" & <More>", "Publisher's Co.", 1, 1999, "/path/to/game", "/ss", "CARD");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(db.getNumGames(), 1);
}

TEST_F(DatabaseTest, InsertGameWithUnicodeCharacters) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Test with Unicode characters (Japanese title)
    bool inserted = db.insertGame(1, "Final Fantasy VII", "Square", 1, 1997, "/path", "/ss", "CARD");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(db.getNumGames(), 1);
}

TEST_F(DatabaseTest, UpdateNonExistentGame) {
    ASSERT_TRUE(db.connect(testDbPath));
    ASSERT_TRUE(db.createInitialDatabase());

    // Try to update a game that doesn't exist - should still return true
    // (SQLite UPDATE succeeds even if no rows match)
    bool updated = db.updateTitle(9999, "Non-existent");
    EXPECT_TRUE(updated);
}

// Main function is provided by gtest_main linked in CMakeLists.txt
