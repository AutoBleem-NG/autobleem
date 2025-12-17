//
// FileUtils Unit Tests
// Tests for file I/O utility functions
//

#include <gtest/gtest.h>
#include "../code/utils/file_utils.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

// Helper function to create a temporary file path
std::string getTempFilePath(const std::string& suffix) {
    // Use /tmp for Linux/Mac, adjust if needed
    static int counter = 0;
    return "/tmp/fileutils_test_" + std::to_string(++counter) + suffix;
}

// Tests for readTextFile
class ReadTextFileTest : public ::testing::Test {
protected:
    std::string tempFile;

    void TearDown() override {
        // Clean up temp file
        std::remove(tempFile.c_str());
    }
};

TEST_F(ReadTextFileTest, ReadsSimpleTextFile) {
    tempFile = getTempFilePath(".txt");

    // Create test file
    {
        std::ofstream out(tempFile);
        out << "Line 1\n";
        out << "Line 2\n";
        out << "Line 3\n";
    }

    // Read and verify
    auto lines = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "Line 1");
    EXPECT_EQ(lines[1], "Line 2");
    EXPECT_EQ(lines[2], "Line 3");
}

TEST_F(ReadTextFileTest, ReadsFileWithoutTrailingNewline) {
    tempFile = getTempFilePath(".txt");

    // Create test file without trailing newline
    {
        std::ofstream out(tempFile);
        out << "Only one line";
    }

    auto lines = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0], "Only one line");
}

TEST_F(ReadTextFileTest, RemovesCRLFWhenFlagTrue) {
    tempFile = getTempFilePath(".txt");

    // Create file with Windows line endings
    {
        std::ofstream out(tempFile);
        out << "Line 1\r\n";
        out << "Line 2\r\n";
    }

    auto lines = FileUtils::readTextFile(tempFile, true);
    EXPECT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "Line 1");
    EXPECT_EQ(lines[1], "Line 2");
}

TEST_F(ReadTextFileTest, PreservesCRLFWhenFlagFalse) {
    tempFile = getTempFilePath(".txt");

    // Create file with Windows line endings
    {
        std::ofstream out(tempFile);
        out << "Line 1\r\n";
        out << "Line 2\r\n";
    }

    auto lines = FileUtils::readTextFile(tempFile, false);
    EXPECT_EQ(lines.size(), 2);
    // Lines may have \r at end when removeCRLF is false
    EXPECT_TRUE(lines[0].find("Line 1") != std::string::npos);
}

TEST_F(ReadTextFileTest, HandlesEmptyFile) {
    tempFile = getTempFilePath(".txt");

    // Create empty file
    {
        std::ofstream out(tempFile);
    }

    auto lines = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(lines.size(), 0);
}

TEST_F(ReadTextFileTest, HandlesNonExistentFile) {
    tempFile = "/tmp/nonexistent_file_that_should_not_exist_12345.txt";

    auto lines = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(lines.size(), 0);
}

TEST_F(ReadTextFileTest, HandlesEmptyLines) {
    tempFile = getTempFilePath(".txt");

    {
        std::ofstream out(tempFile);
        out << "Line 1\n";
        out << "\n";
        out << "Line 3\n";
    }

    auto lines = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "Line 1");
    EXPECT_EQ(lines[1], "");
    EXPECT_EQ(lines[2], "Line 3");
}

// Tests for writeTextFile
class WriteTextFileTest : public ::testing::Test {
protected:
    std::string tempFile;

    void TearDown() override {
        std::remove(tempFile.c_str());
    }
};

TEST_F(WriteTextFileTest, WritesSimpleTextFile) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> lines = {"Line 1", "Line 2", "Line 3"};
    bool success = FileUtils::writeTextFile(tempFile, lines);

    EXPECT_TRUE(success);

    // Verify file contents
    auto readBack = FileUtils::readTextFile(tempFile, false);
    EXPECT_EQ(readBack.size(), 3);
}

TEST_F(WriteTextFileTest, WritesWithLineEndings) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> lines = {"Line 1", "Line 2"};
    bool success = FileUtils::writeTextFile(tempFile, lines, true);

    EXPECT_TRUE(success);

    // Check that file has newlines
    std::ifstream in(tempFile);
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

    // Should have newlines
    EXPECT_NE(content.find('\n'), std::string::npos);
}

TEST_F(WriteTextFileTest, WritesWithoutLineEndings) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> lines = {"Line 1", "Line 2"};
    bool success = FileUtils::writeTextFile(tempFile, lines, false);

    EXPECT_TRUE(success);
}

TEST_F(WriteTextFileTest, OverwritesExistingFile) {
    tempFile = getTempFilePath(".txt");

    // Write initial content
    {
        std::ofstream out(tempFile);
        out << "Original content that should be replaced\n";
    }

    // Overwrite with new content
    std::vector<std::string> lines = {"New line 1", "New line 2"};
    bool success = FileUtils::writeTextFile(tempFile, lines);

    EXPECT_TRUE(success);

    // Verify new content
    auto readBack = FileUtils::readTextFile(tempFile, false);
    EXPECT_EQ(readBack.size(), 2);
}

TEST_F(WriteTextFileTest, WritesEmptyVector) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> lines;
    bool success = FileUtils::writeTextFile(tempFile, lines);

    EXPECT_TRUE(success);

    auto readBack = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(readBack.size(), 0);
}

TEST_F(WriteTextFileTest, WritesWithSpecialCharacters) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> lines = {
        "Line with \"quotes\"",
        "Line with 'apostrophes'",
        "Line with special chars: !@#$%^&*()"
    };
    bool success = FileUtils::writeTextFile(tempFile, lines);

    EXPECT_TRUE(success);

    auto readBack = FileUtils::readTextFile(tempFile, false);
    EXPECT_EQ(readBack.size(), 3);
}

// Tests for getlineRemoveCR
class GetlineRemoveCRTest : public ::testing::Test {
protected:
    std::string tempFile;

    void TearDown() override {
        std::remove(tempFile.c_str());
    }
};

TEST_F(GetlineRemoveCRTest, RemovesCR) {
    tempFile = getTempFilePath(".txt");

    // Create file with Windows line ending
    {
        std::ofstream out(tempFile, std::ios::binary);
        out << "Line with CR\r\n";
        out << "Another line\r\n";
    }

    std::ifstream in(tempFile);
    std::string line;

    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "Line with CR");
    EXPECT_EQ(line.back(), 'R');  // Should not have CR

    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "Another line");
}

TEST_F(GetlineRemoveCRTest, HandlesLFOnly) {
    tempFile = getTempFilePath(".txt");

    // Create file with Unix line ending
    {
        std::ofstream out(tempFile);
        out << "Line with LF\n";
    }

    std::ifstream in(tempFile);
    std::string line;

    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "Line with LF");
}

TEST_F(GetlineRemoveCRTest, HandlesMixedLineEndings) {
    tempFile = getTempFilePath(".txt");

    {
        std::ofstream out(tempFile, std::ios::binary);
        out << "Windows line\r\n";
        out << "Unix line\n";
        out << "Mac line\r";
    }

    std::ifstream in(tempFile);
    std::string line;

    // Windows line
    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "Windows line");

    // Unix line
    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "Unix line");
}

TEST_F(GetlineRemoveCRTest, HandlesEmptyLine) {
    tempFile = getTempFilePath(".txt");

    {
        std::ofstream out(tempFile, std::ios::binary);
        out << "\r\n";
        out << "Not empty\r\n";
    }

    std::ifstream in(tempFile);
    std::string line;

    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "");

    FileUtils::getlineRemoveCR(in, line);
    EXPECT_EQ(line, "Not empty");
}

TEST_F(GetlineRemoveCRTest, ReturnsReferenceCorrectly) {
    tempFile = getTempFilePath(".txt");

    {
        std::ofstream out(tempFile);
        out << "Test\n";
    }

    std::ifstream in(tempFile);
    std::string line;

    std::istream& result = FileUtils::getlineRemoveCR(in, line);
    EXPECT_TRUE(&result == &in);  // Should return the input stream reference
}

// Integration tests
class FileUtilsIntegrationTest : public ::testing::Test {
protected:
    std::string tempFile;

    void TearDown() override {
        std::remove(tempFile.c_str());
    }
};

TEST_F(FileUtilsIntegrationTest, WriteAndReadRoundTrip) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> originalLines = {
        "First line",
        "Second line",
        "Third line with special chars: @#$%"
    };

    // Write
    bool writeSuccess = FileUtils::writeTextFile(tempFile, originalLines, true);
    EXPECT_TRUE(writeSuccess);

    // Read back
    auto readLines = FileUtils::readTextFile(tempFile, false);
    EXPECT_EQ(readLines.size(), originalLines.size());

    for (size_t i = 0; i < originalLines.size(); ++i) {
        EXPECT_EQ(readLines[i], originalLines[i]);
    }
}

TEST_F(FileUtilsIntegrationTest, LargeFileHandling) {
    tempFile = getTempFilePath(".txt");

    // Create a file with many lines
    std::vector<std::string> lines;
    for (int i = 0; i < 1000; ++i) {
        lines.push_back("Line " + std::to_string(i));
    }

    bool writeSuccess = FileUtils::writeTextFile(tempFile, lines, true);
    EXPECT_TRUE(writeSuccess);

    auto readLines = FileUtils::readTextFile(tempFile);
    EXPECT_EQ(readLines.size(), 1000);
    EXPECT_EQ(readLines[0], "Line 0");
    EXPECT_EQ(readLines[999], "Line 999");
}
