// Unit tests for BinaryIO::Reader
#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>

#include "../code/io/binary_reader.h"

class BinaryReaderTest : public ::testing::Test {
protected:
    std::string testFile;

    void SetUp() override {
        testFile = "/tmp/binary_reader_test.bin";
    }

    void TearDown() override {
        std::remove(testFile.c_str());
    }

    void writeTestFile(const std::vector<uint8_t>& data) {
        std::ofstream file(testFile, std::ios::binary);
        for (uint8_t byte : data) {
            file.put(static_cast<char>(byte));
        }
    }
};

// ============================================================================
// readByte() Tests
// ============================================================================

TEST_F(BinaryReaderTest, ReadByte) {
    writeTestFile({0x42, 0xFF, 0x00});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readByte(), 0x42);
    EXPECT_EQ(reader.readByte(), 0xFF);
    EXPECT_EQ(reader.readByte(), 0x00);
}

// ============================================================================
// readDword() Tests (Little-Endian)
// ============================================================================

TEST_F(BinaryReaderTest, ReadDword) {
    // 0x12345678 in little-endian: 78 56 34 12
    writeTestFile({0x78, 0x56, 0x34, 0x12});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readDword(), 0x12345678U);
}

TEST_F(BinaryReaderTest, ReadDwordZeroAndMax) {
    writeTestFile({0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readDword(), 0x00000000U);
    EXPECT_EQ(reader.readDword(), 0xFFFFFFFFU);
}

// ============================================================================
// readFixedString() Tests
// ============================================================================

TEST_F(BinaryReaderTest, ReadFixedString) {
    writeTestFile({'h', 'e', 'l', 'l', 'o'});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readFixedString(5), "hello");
}

TEST_F(BinaryReaderTest, ReadFixedStringWithNulls) {
    writeTestFile({'h', 'e', 'l', 'l', 'o', '\0', '\0', '\0'});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    // readFixedString reads the bytes but string stops at null
    std::string result = reader.readFixedString(8);
    EXPECT_EQ(result, "hello");
}

// ============================================================================
// readNullTerminatedString() Tests
// ============================================================================

TEST_F(BinaryReaderTest, ReadNullTerminatedString) {
    writeTestFile({'h', 'e', 'l', 'l', 'o', '\0', 'w', 'o', 'r', 'l', 'd', '\0'});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readNullTerminatedString(), "hello");
    EXPECT_EQ(reader.readNullTerminatedString(), "world");
}

TEST_F(BinaryReaderTest, ReadNullTerminatedStringEmpty) {
    writeTestFile({'\0'});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readNullTerminatedString(), "");
}

// ============================================================================
// skipZeros() Tests
// ============================================================================

TEST_F(BinaryReaderTest, SkipZeros) {
    writeTestFile({0x00, 0x00, 0x00, 0x42});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    reader.skipZeros();
    EXPECT_EQ(reader.readByte(), 0x42);
}

TEST_F(BinaryReaderTest, SkipZerosNoZeros) {
    writeTestFile({0x42, 0x43});
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    reader.skipZeros();  // Should not skip anything
    EXPECT_EQ(reader.readByte(), 0x42);
}

// ============================================================================
// Integration Test
// ============================================================================

TEST_F(BinaryReaderTest, ReadMixedData) {
    std::vector<uint8_t> data;
    data.push_back(0x01);  // byte
    data.push_back(0x78); data.push_back(0x56); data.push_back(0x34); data.push_back(0x12);  // dword
    data.push_back('t'); data.push_back('e'); data.push_back('s'); data.push_back('t'); data.push_back('\0');  // string

    writeTestFile(data);
    std::ifstream file(testFile, std::ios::binary);
    BinaryIO::Reader reader(&file);

    EXPECT_EQ(reader.readByte(), 0x01);
    EXPECT_EQ(reader.readDword(), 0x12345678U);
    EXPECT_EQ(reader.readNullTerminatedString(), "test");
}
