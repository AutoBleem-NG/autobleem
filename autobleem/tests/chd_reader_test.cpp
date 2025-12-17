// Unit tests for CHDReader and CDReader classes
//
// These tests verify essential disc image reading functionality:
// - Opening valid/invalid CHD and ISO files
// - Reading sector data correctly
// - Calibration (finding ISO9660 signature)
// - Error handling
// - Memory management
//
// Test files required in autobleem/tests/test_data/:
// - test.chd - A small PlayStation CHD disc image
// - test.bin - A small BIN disc image
//
// The TEST_DATA_DIR macro is defined by CMake at compile time.

#include <gtest/gtest.h>
#include "engine/cd_reader.h"
#include <fstream>
#include <cstring>

// Test fixture for CHDReader tests
class CHDReaderTest : public ::testing::Test {
  protected:
    CHDReader reader;
    std::string testDataDir;

    void SetUp() override {
        // TEST_DATA_DIR is defined by CMake at compile time
        testDataDir = TEST_DATA_DIR;
    }

    void TearDown() override {
        if (reader.isOpen()) {
            reader.closeImage();
        }
    }

    bool hasTestFile(const std::string &filename) {
        std::ifstream f(testDataDir + filename);
        return f.good();
    }
};

// Test fixture for CDReader (ISO/BIN) tests
class CDReaderTest : public ::testing::Test {
  protected:
    CDReader reader;
    std::string testDataDir;

    void SetUp() override {
        // TEST_DATA_DIR is defined by CMake at compile time
        testDataDir = TEST_DATA_DIR;
    }

    void TearDown() override {
        if (reader.isOpen()) {
            reader.closeImage();
        }
    }

    bool hasTestFile(const std::string &filename) {
        std::ifstream f(testDataDir + filename);
        return f.good();
    }
};

//=============================================================================
// Error Handling Tests
//=============================================================================

TEST_F(CHDReaderTest, OpenNonExistentFile) {
    int result = reader.openImage("/nonexistent/path/missing.chd");
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(reader.isOpen());
}

TEST_F(CHDReaderTest, OpenInvalidFile) {
    // Create a file with invalid CHD data
    std::string tempFile = "/tmp/invalid_test.chd";
    std::ofstream f(tempFile);
    f << "This is not a valid CHD file, just garbage data";
    f.close();

    int result = reader.openImage(tempFile);
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(reader.isOpen());

    std::remove(tempFile.c_str());
}

TEST_F(CDReaderTest, OpenNonExistentISO) {
    int result = reader.openImage("/nonexistent/path/missing.iso");
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(reader.isOpen());
}

//=============================================================================
// Basic Functionality Tests
//=============================================================================

TEST_F(CHDReaderTest, BufferManagement) {
    EXPECT_EQ(reader.getBuffer(), nullptr) << "Buffer should be null before opening";

    if (hasTestFile("test.chd")) {
        int result = reader.openImage(testDataDir + "test.chd");
        if (result == 0) {
            EXPECT_NE(reader.getBuffer(), nullptr) << "Buffer should be allocated after opening";
            reader.closeImage();
            // Note: Buffer is deleted in closeImage, no need to check for null after
        }
    }
}

TEST(CDReaderBasicTest, SectorNavigation) {
    CDReader reader;

    // Test sector position tracking
    reader.setCurrentSector(16);
    EXPECT_EQ(reader.getCurrentSector(), 16);

    reader.setSectorpos(100);
    EXPECT_EQ(reader.getSectorPos(), 100);

    // Test navigation
    reader.setCurrentSector(0);
    reader.setSectorpos(0);
    EXPECT_EQ(reader.getCurrentSector(), 0);
    EXPECT_EQ(reader.getSectorPos(), 0);
}

TEST(CDReaderBasicTest, OffsetManagement) {
    CDReader reader;

    reader.setOffset(0);
    EXPECT_EQ(reader.getOffset(), 0);

    reader.setOffset(16);
    EXPECT_EQ(reader.getOffset(), 16);

    reader.setOffset(24);
    EXPECT_EQ(reader.getOffset(), 24);
}

//=============================================================================
// CHD Integration Tests (require test.chd)
//=============================================================================

TEST_F(CHDReaderTest, OpenValidCHD) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0) << "Failed to open valid CHD file";
    EXPECT_TRUE(reader.isOpen());
    EXPECT_NE(reader.getBuffer(), nullptr);

    reader.closeImage();
    EXPECT_FALSE(reader.isOpen());
}

TEST_F(CHDReaderTest, ReadSectorData) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0);

    // Select sector 16 (standard location for ISO9660 primary volume descriptor)
    reader.selectSector(16);
    EXPECT_EQ(reader.getCurrentSector(), 16);
    EXPECT_EQ(reader.getSectorPos(), 0);

    // Read some data
    unsigned char byte1 = reader.readChar();
    EXPECT_EQ(reader.getSectorPos(), 1);

    unsigned char byte2 = reader.readChar();
    EXPECT_EQ(reader.getSectorPos(), 2);

    // Position should advance
    EXPECT_NE(byte1, 255) << "Read data should be valid (not likely all 0xFF)";

    reader.closeImage();
}

TEST_F(CHDReaderTest, CalibrationSucceeds) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0) << "Opening CHD should succeed and calibration should find CD001";

    // If we got here, calibration found the CD001 signature
    // (otherwise openImage would have returned -1)
    EXPECT_TRUE(reader.isOpen());
    EXPECT_GE(reader.getOffset(), 0);

    reader.closeImage();
}

TEST_F(CHDReaderTest, EndOfStreamDetection) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0);

    // At start of disc, should not be at end
    EXPECT_FALSE(reader.endStream());

    // Read a sector
    reader.selectSector(16);
    EXPECT_FALSE(reader.endStream());

    reader.closeImage();
}

TEST_F(CHDReaderTest, MultipleOpenClose) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    std::string path = testDataDir + "test.chd";

    // Open and close multiple times
    for (int i = 0; i < 3; i++) {
        int result = reader.openImage(path);
        EXPECT_EQ(result, 0) << "Open attempt " << (i + 1) << " failed";
        EXPECT_TRUE(reader.isOpen());

        reader.closeImage();
        EXPECT_FALSE(reader.isOpen());
    }
}

//=============================================================================
// ISO Integration Tests (require test.iso)
//=============================================================================

TEST_F(CDReaderTest, OpenValidISO) {
    if (!hasTestFile("test.bin")) {
        GTEST_SKIP() << "Test file test.bin not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.bin");
    ASSERT_EQ(result, 0) << "Failed to open valid ISO file";
    EXPECT_TRUE(reader.isOpen());
    EXPECT_NE(reader.getBuffer(), nullptr);

    reader.closeImage();
    EXPECT_FALSE(reader.isOpen());
}

TEST_F(CDReaderTest, ISOCalibrationFindsCD001) {
    if (!hasTestFile("test.bin")) {
        GTEST_SKIP() << "Test file test.bin not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.bin");
    ASSERT_EQ(result, 0) << "Calibration should find CD001 signature";
    EXPECT_TRUE(reader.isOpen());

    // Calibration succeeded, so offset should be set
    int offset = reader.getOffset();
    EXPECT_GE(offset, 0);

    reader.closeImage();
}

TEST_F(CDReaderTest, ISOReadSectorData) {
    if (!hasTestFile("test.bin")) {
        GTEST_SKIP() << "Test file test.bin not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.bin");
    ASSERT_EQ(result, 0);

    // Read sector 16 (primary volume descriptor)
    reader.selectSector(16);
    EXPECT_EQ(reader.getCurrentSector(), 16);

    // Read some bytes
    unsigned char b1 = reader.readChar();
    unsigned char b2 = reader.readChar();
    unsigned char b3 = reader.readChar();

    // Should have read valid data
    EXPECT_EQ(reader.getSectorPos(), 3);

    reader.closeImage();
}

//=============================================================================
// Data Reading Tests
//=============================================================================

TEST_F(CHDReaderTest, ReadString) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0);

    reader.selectSector(16);
    std::string str = reader.readString(5);
    EXPECT_EQ(str.length(), 5);

    reader.closeImage();
}

TEST_F(CHDReaderTest, ReadDword) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0);

    reader.selectSector(16);
    unsigned long dword = reader.readDword();

    // Should have read 4 bytes
    EXPECT_EQ(reader.getSectorPos(), 4);

    reader.closeImage();
}

TEST_F(CHDReaderTest, ForwardAndReverse) {
    if (!hasTestFile("test.chd")) {
        GTEST_SKIP() << "Test file test.chd not found in " << testDataDir;
    }

    int result = reader.openImage(testDataDir + "test.chd");
    ASSERT_EQ(result, 0);

    reader.selectSector(16);
    EXPECT_EQ(reader.getSectorPos(), 0);

    // Move forward 10 bytes
    reader.ffd(10);
    EXPECT_EQ(reader.getSectorPos(), 10);

    // Move back 5 bytes
    reader.rev(5);
    EXPECT_EQ(reader.getSectorPos(), 5);

    reader.closeImage();
}

//=============================================================================
// Comparison Test: CHD vs ISO
//=============================================================================

TEST(CHDvsISOTest, BothFormatsReadSameData) {
    // TEST_DATA_DIR is defined by CMake at compile time
    std::string testDataDir = TEST_DATA_DIR;

    // Check if both test files exist
    std::ifstream chd_check(testDataDir + "test.chd");
    std::ifstream bin_check(testDataDir + "test.bin");

    if (!chd_check.good() || !bin_check.good()) {
        GTEST_SKIP() << "Both test.chd and test.bin required for comparison test";
    }

    CHDReader chd_reader;
    CDReader bin_reader;

    int chd_result = chd_reader.openImage(testDataDir + "test.chd");
    int bin_result = bin_reader.openImage(testDataDir + "test.bin");

    ASSERT_EQ(chd_result, 0) << "CHD should open successfully";
    ASSERT_EQ(bin_result, 0) << "BIN should open successfully";

    // Read first few bytes from sector 16 in both formats
    chd_reader.selectSector(16);
    bin_reader.selectSector(16);

    for (int i = 0; i < 10; i++) {
        unsigned char chd_byte = chd_reader.readChar();
        unsigned char bin_byte = bin_reader.readChar();

        EXPECT_EQ(chd_byte, bin_byte) << "Byte " << i << " differs between CHD and BIN";
    }

    chd_reader.closeImage();
    bin_reader.closeImage();
}

// Main function is provided by gtest_main linked in CMakeLists.txt
