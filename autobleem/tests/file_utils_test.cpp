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
std::string getTempFilePath(const std::string &suffix) {
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

    void TearDown() override { std::remove(tempFile.c_str()); }
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
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

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

    std::vector<std::string> lines = {"Line with \"quotes\"", "Line with 'apostrophes'",
                                      "Line with special chars: !@#$%^&*()"};
    bool success = FileUtils::writeTextFile(tempFile, lines);

    EXPECT_TRUE(success);

    auto readBack = FileUtils::readTextFile(tempFile, false);
    EXPECT_EQ(readBack.size(), 3);
}

// Tests for getlineRemoveCR
class GetlineRemoveCRTest : public ::testing::Test {
  protected:
    std::string tempFile;

    void TearDown() override { std::remove(tempFile.c_str()); }
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
    EXPECT_EQ(line.back(), 'R'); // Should not have CR

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

    std::istream &result = FileUtils::getlineRemoveCR(in, line);
    EXPECT_TRUE(&result == &in); // Should return the input stream reference
}

// Integration tests
class FileUtilsIntegrationTest : public ::testing::Test {
  protected:
    std::string tempFile;

    void TearDown() override { std::remove(tempFile.c_str()); }
};

TEST_F(FileUtilsIntegrationTest, WriteAndReadRoundTrip) {
    tempFile = getTempFilePath(".txt");

    std::vector<std::string> originalLines = {"First line", "Second line", "Third line with special chars: @#$%"};

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

// Tests for createDirRecursive
class CreateDirRecursiveTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override { tempBase = "/tmp/fileutils_test_dir_" + std::to_string(rand()); }

    void TearDown() override {
        // Clean up created directories
        FileUtils::removeDirAndContents(tempBase);
    }
};

TEST_F(CreateDirRecursiveTest, CreatesSingleDirectory) {
    std::string dir = tempBase;

    bool success = FileUtils::createDirRecursive(dir);
    EXPECT_TRUE(success);
    EXPECT_TRUE(FileUtils::exists(dir));
    EXPECT_TRUE(FileUtils::isDirectory(dir));
}

TEST_F(CreateDirRecursiveTest, CreatesNestedDirectories) {
    std::string dir = tempBase + "/level1/level2/level3";

    bool success = FileUtils::createDirRecursive(dir);
    EXPECT_TRUE(success);
    EXPECT_TRUE(FileUtils::exists(dir));
    EXPECT_TRUE(FileUtils::isDirectory(dir));
    EXPECT_TRUE(FileUtils::isDirectory(tempBase + "/level1"));
    EXPECT_TRUE(FileUtils::isDirectory(tempBase + "/level1/level2"));
}

TEST_F(CreateDirRecursiveTest, SucceedsIfAlreadyExists) {
    std::string dir = tempBase;

    // Create it first
    bool success1 = FileUtils::createDirRecursive(dir);
    EXPECT_TRUE(success1);

    // Should succeed again (idempotent)
    bool success2 = FileUtils::createDirRecursive(dir);
    EXPECT_TRUE(success2);
}

TEST_F(CreateDirRecursiveTest, HandlesEmptyPath) {
    bool success = FileUtils::createDirRecursive("");
    EXPECT_FALSE(success);
}

// Tests for ensureParentDirExists
class EnsureParentDirExistsTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override { tempBase = "/tmp/fileutils_test_parent_" + std::to_string(rand()); }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(EnsureParentDirExistsTest, CreatesParentDirectoryForFile) {
    std::string filePath = tempBase + "/subdir/myfile.txt";

    bool success = FileUtils::ensureParentDirExists(filePath);
    EXPECT_TRUE(success);

    // Parent directory should exist
    EXPECT_TRUE(FileUtils::exists(tempBase + "/subdir"));
    EXPECT_TRUE(FileUtils::isDirectory(tempBase + "/subdir"));
}

TEST_F(EnsureParentDirExistsTest, CreatesNestedParentDirectories) {
    std::string filePath = tempBase + "/a/b/c/myfile.log";

    bool success = FileUtils::ensureParentDirExists(filePath);
    EXPECT_TRUE(success);

    EXPECT_TRUE(FileUtils::isDirectory(tempBase + "/a"));
    EXPECT_TRUE(FileUtils::isDirectory(tempBase + "/a/b"));
    EXPECT_TRUE(FileUtils::isDirectory(tempBase + "/a/b/c"));
}

TEST_F(EnsureParentDirExistsTest, SucceedsForFileInCurrentDir) {
    // File in current directory - no parent to create
    bool success = FileUtils::ensureParentDirExists("myfile.txt");
    EXPECT_TRUE(success);
}

TEST_F(EnsureParentDirExistsTest, SucceedsIfParentAlreadyExists) {
    // Create parent first
    FileUtils::createDirRecursive(tempBase);

    std::string filePath = tempBase + "/myfile.txt";
    bool success = FileUtils::ensureParentDirExists(filePath);
    EXPECT_TRUE(success);
}

// ========================================
// Tests for File Operations
// ========================================

class FileOperationsTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override { tempBase = "/tmp/fileops_test_" + std::to_string(rand()); }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(FileOperationsTest, CreateDirCreatesDirectory) {
    std::string dir = tempBase;
    bool success = FileUtils::createDir(dir);
    EXPECT_TRUE(success);
    EXPECT_TRUE(FileUtils::exists(dir));
    EXPECT_TRUE(FileUtils::isDirectory(dir));
}

TEST_F(FileOperationsTest, CreateDirFailsIfAlreadyExists) {
    std::string dir = tempBase;
    FileUtils::createDir(dir);
    // Creating same directory again should fail
    bool success = FileUtils::createDir(dir);
    EXPECT_FALSE(success);
}

TEST_F(FileOperationsTest, CopyFilesCopiesContent) {
    FileUtils::createDir(tempBase);
    std::string source = tempBase + "/source.txt";
    std::string dest = tempBase + "/dest.txt";

    // Create source file
    std::vector<std::string> content = {"Line 1", "Line 2", "Line 3"};
    FileUtils::writeTextFile(source, content, true);

    // Copy file
    bool success = FileUtils::copy(source, dest);
    EXPECT_TRUE(success);
    EXPECT_TRUE(FileUtils::exists(dest));

    // Verify content
    auto readContent = FileUtils::readTextFile(dest);
    EXPECT_EQ(readContent.size(), 3);
    EXPECT_EQ(readContent[0], "Line 1");
}

TEST_F(FileOperationsTest, CopyFileAliasWorks) {
    FileUtils::createDir(tempBase);
    std::string source = tempBase + "/source.txt";
    std::string dest = tempBase + "/dest.txt";

    std::vector<std::string> content = {"Test content"};
    FileUtils::writeTextFile(source, content, true);

    bool success = FileUtils::copyFile(source, dest);
    EXPECT_TRUE(success);
    EXPECT_TRUE(FileUtils::exists(dest));
}

TEST_F(FileOperationsTest, RemoveFileDeletesFile) {
    FileUtils::createDir(tempBase);
    std::string file = tempBase + "/to_delete.txt";

    std::vector<std::string> content = {"Delete me"};
    FileUtils::writeTextFile(file, content, true);
    EXPECT_TRUE(FileUtils::exists(file));

    bool success = FileUtils::removeFile(file);
    EXPECT_TRUE(success);
    EXPECT_FALSE(FileUtils::exists(file));
}

TEST_F(FileOperationsTest, RemoveFileReturnsFalseForNonexistent) {
    std::string file = "/tmp/nonexistent_file_12345.txt";
    bool success = FileUtils::removeFile(file);
    EXPECT_FALSE(success);
}

TEST_F(FileOperationsTest, RenameFileMovesFile) {
    FileUtils::createDir(tempBase);
    std::string oldPath = tempBase + "/old_name.txt";
    std::string newPath = tempBase + "/new_name.txt";

    std::vector<std::string> content = {"Rename me"};
    FileUtils::writeTextFile(oldPath, content, true);

    bool success = FileUtils::renameFile(oldPath, newPath);
    EXPECT_TRUE(success);
    EXPECT_FALSE(FileUtils::exists(oldPath));
    EXPECT_TRUE(FileUtils::exists(newPath));
}

TEST_F(FileOperationsTest, ExistsReturnsTrueForFile) {
    FileUtils::createDir(tempBase);
    std::string file = tempBase + "/exists.txt";
    std::vector<std::string> content = {"I exist"};
    FileUtils::writeTextFile(file, content, true);

    EXPECT_TRUE(FileUtils::exists(file));
}

TEST_F(FileOperationsTest, ExistsReturnsTrueForDirectory) {
    FileUtils::createDir(tempBase);
    EXPECT_TRUE(FileUtils::exists(tempBase));
}

TEST_F(FileOperationsTest, ExistsReturnsFalseForNonexistent) {
    EXPECT_FALSE(FileUtils::exists("/tmp/nonexistent_path_12345"));
}

TEST_F(FileOperationsTest, IsDirectoryReturnsTrueForDir) {
    FileUtils::createDir(tempBase);
    EXPECT_TRUE(FileUtils::isDirectory(tempBase));
}

TEST_F(FileOperationsTest, IsDirectoryReturnsFalseForFile) {
    FileUtils::createDir(tempBase);
    std::string file = tempBase + "/file.txt";
    std::vector<std::string> content = {"Not a dir"};
    FileUtils::writeTextFile(file, content, true);

    EXPECT_FALSE(FileUtils::isDirectory(file));
}

TEST_F(FileOperationsTest, IsDirectoryReturnsFalseForNonexistent) {
    EXPECT_FALSE(FileUtils::isDirectory("/tmp/nonexistent_12345"));
}

// ========================================
// Tests for Directory Listing
// ========================================

class DirectoryListingTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override {
        tempBase = "/tmp/dirlist_test_" + std::to_string(rand());
        FileUtils::createDir(tempBase);
    }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(DirectoryListingTest, DiruListsFilesAndDirs) {
    // Create files and directories
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file1.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/file2.txt", content, true);
    FileUtils::createDir(tempBase + "/subdir1");
    FileUtils::createDir(tempBase + "/subdir2");

    auto entries = FileUtils::diru(tempBase);

    EXPECT_EQ(entries.size(), 4);
}

TEST_F(DirectoryListingTest, DiruExcludesHiddenFiles) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/.hidden", content, true);
    FileUtils::writeTextFile(tempBase + "/visible.txt", content, true);

    auto entries = FileUtils::diru(tempBase);

    EXPECT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].name, "visible.txt");
}

TEST_F(DirectoryListingTest, DiruDirsOnlyReturnsOnlyDirs) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file.txt", content, true);
    FileUtils::createDir(tempBase + "/subdir1");
    FileUtils::createDir(tempBase + "/subdir2");

    auto entries = FileUtils::diru_DirsOnly(tempBase);

    EXPECT_EQ(entries.size(), 2);
    for (const auto &entry : entries) {
        EXPECT_TRUE(entry.isDir);
    }
}

TEST_F(DirectoryListingTest, DiruFilesOnlyReturnsOnlyFiles) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file1.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/file2.txt", content, true);
    FileUtils::createDir(tempBase + "/subdir");

    auto entries = FileUtils::diru_FilesOnly(tempBase);

    EXPECT_EQ(entries.size(), 2);
    for (const auto &entry : entries) {
        EXPECT_FALSE(entry.isDir);
    }
}

TEST_F(DirectoryListingTest, DiruSortsByName) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/zebra.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/apple.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/mango.txt", content, true);

    auto entries = FileUtils::diru(tempBase);

    EXPECT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].name, "apple.txt");
    EXPECT_EQ(entries[1].name, "mango.txt");
    EXPECT_EQ(entries[2].name, "zebra.txt");
}

TEST_F(DirectoryListingTest, DiruHandlesEmptyDirectory) {
    auto entries = FileUtils::diru(tempBase);
    EXPECT_EQ(entries.size(), 0);
}

TEST_F(DirectoryListingTest, DirIncludesDotEntries) {
    auto entries = FileUtils::dir(tempBase);

    // Should include . and ..
    bool hasDot = false;
    bool hasDotDot = false;
    for (const auto &entry : entries) {
        if (entry.name == ".")
            hasDot = true;
        if (entry.name == "..")
            hasDotDot = true;
    }
    EXPECT_TRUE(hasDot);
    EXPECT_TRUE(hasDotDot);
}

// ========================================
// Tests for Extension Handling
// ========================================

class ExtensionHandlingTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override {
        tempBase = "/tmp/ext_test_" + std::to_string(rand());
        FileUtils::createDir(tempBase);
    }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(ExtensionHandlingTest, RemoveDotFromExtensionRemovesDot) {
    EXPECT_EQ(FileUtils::removeDotFromExtension(".txt"), "txt");
    EXPECT_EQ(FileUtils::removeDotFromExtension(".cpp"), "cpp");
}

TEST_F(ExtensionHandlingTest, RemoveDotFromExtensionHandlesNoDot) {
    EXPECT_EQ(FileUtils::removeDotFromExtension("txt"), "txt");
}

TEST_F(ExtensionHandlingTest, RemoveDotFromExtensionHandlesEmpty) {
    EXPECT_EQ(FileUtils::removeDotFromExtension(""), "");
}

TEST_F(ExtensionHandlingTest, AddDotToExtensionAddsDot) {
    EXPECT_EQ(FileUtils::addDotToExtension("txt"), ".txt");
    EXPECT_EQ(FileUtils::addDotToExtension("cpp"), ".cpp");
}

TEST_F(ExtensionHandlingTest, AddDotToExtensionPreservesDot) {
    EXPECT_EQ(FileUtils::addDotToExtension(".txt"), ".txt");
}

TEST_F(ExtensionHandlingTest, AddDotToExtensionHandlesEmpty) {
    EXPECT_EQ(FileUtils::addDotToExtension(""), "");
}

TEST_F(ExtensionHandlingTest, MatchExtensionMatchesExact) {
    EXPECT_TRUE(FileUtils::matchExtension("file.txt", "txt"));
    EXPECT_TRUE(FileUtils::matchExtension("file.txt", ".txt"));
}

TEST_F(ExtensionHandlingTest, MatchExtensionIsCaseInsensitive) {
    EXPECT_TRUE(FileUtils::matchExtension("file.TXT", "txt"));
    EXPECT_TRUE(FileUtils::matchExtension("file.txt", "TXT"));
}

TEST_F(ExtensionHandlingTest, MatchExtensionReturnsFalseForMismatch) {
    EXPECT_FALSE(FileUtils::matchExtension("file.txt", "cpp"));
}

TEST_F(ExtensionHandlingTest, MatchExtensionReturnsFalseForShortPath) {
    EXPECT_FALSE(FileUtils::matchExtension("abc", "txt"));
}

TEST_F(ExtensionHandlingTest, GetFilesWithExtensionFiltersCorrectly) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file1.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/file2.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/file3.cpp", content, true);
    FileUtils::writeTextFile(tempBase + "/file4.bin", content, true);

    auto allEntries = FileUtils::diru(tempBase);
    std::vector<std::string> extensions = {"txt", "cpp"};

    auto filtered = FileUtils::getFilesWithExtension(tempBase, allEntries, extensions);

    EXPECT_EQ(filtered.size(), 3);
}

TEST_F(ExtensionHandlingTest, GetFilesWithExtensionExcludesDirectories) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file.txt", content, true);
    FileUtils::createDir(tempBase + "/subdir.txt"); // Directory with extension-like name

    auto allEntries = FileUtils::diru(tempBase);
    std::vector<std::string> extensions = {"txt"};

    auto filtered = FileUtils::getFilesWithExtension(tempBase, allEntries, extensions);

    EXPECT_EQ(filtered.size(), 1);
    EXPECT_EQ(filtered[0].name, "file.txt");
}

TEST_F(ExtensionHandlingTest, FindFirstFileFindsFile) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/other.cpp", content, true);
    FileUtils::writeTextFile(tempBase + "/target.txt", content, true);

    std::string found = FileUtils::findFirstFile("txt", tempBase);
    EXPECT_EQ(found, "target.txt");
}

TEST_F(ExtensionHandlingTest, FindFirstFileReturnsEmptyIfNotFound) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file.cpp", content, true);

    std::string found = FileUtils::findFirstFile("txt", tempBase);
    EXPECT_EQ(found, "");
}

// ========================================
// Tests for Game-Specific Functions
// ========================================

class GameSpecificTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override {
        tempBase = "/tmp/game_test_" + std::to_string(rand());
        FileUtils::createDir(tempBase);
    }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(GameSpecificTest, IsPBPFileRecognizesPBP) {
    EXPECT_TRUE(FileUtils::isPBPFile("game.pbp"));
    EXPECT_TRUE(FileUtils::isPBPFile("game.PBP"));
    EXPECT_TRUE(FileUtils::isPBPFile("/path/to/game.pbp"));
}

TEST_F(GameSpecificTest, IsPBPFileRejectsNonPBP) {
    EXPECT_FALSE(FileUtils::isPBPFile("game.bin"));
    EXPECT_FALSE(FileUtils::isPBPFile("game.txt"));
}

TEST_F(GameSpecificTest, IsPBPFileRejectsShortPath) {
    EXPECT_FALSE(FileUtils::isPBPFile("abc"));
    EXPECT_FALSE(FileUtils::isPBPFile(""));
}

TEST_F(GameSpecificTest, IsAGameFileRecognizesBin) {
    EXPECT_TRUE(FileUtils::isAGameFile("game.bin"));
    EXPECT_TRUE(FileUtils::isAGameFile("game.BIN"));
}

TEST_F(GameSpecificTest, IsAGameFileRecognizesPBP) {
    EXPECT_TRUE(FileUtils::isAGameFile("game.pbp"));
    EXPECT_TRUE(FileUtils::isAGameFile("game.PBP"));
}

TEST_F(GameSpecificTest, IsAGameFileRecognizesIMG) {
    EXPECT_TRUE(FileUtils::isAGameFile("game.img"));
    EXPECT_TRUE(FileUtils::isAGameFile("game.IMG"));
}

TEST_F(GameSpecificTest, IsAGameFileRecognizesCHD) {
    EXPECT_TRUE(FileUtils::isAGameFile("game.chd"));
    EXPECT_TRUE(FileUtils::isAGameFile("game.CHD"));
}

TEST_F(GameSpecificTest, IsAGameFileRejectsNonGame) {
    EXPECT_FALSE(FileUtils::isAGameFile("readme.txt"));
    EXPECT_FALSE(FileUtils::isAGameFile("cover.png"));
    EXPECT_FALSE(FileUtils::isAGameFile("game.cue"));
}

TEST_F(GameSpecificTest, GetGameFileImageTypeReturnsBin) {
    EXPECT_EQ(FileUtils::getGameFileImageType("game.bin"), IMAGE_BIN);
}

TEST_F(GameSpecificTest, GetGameFileImageTypeReturnsPBP) {
    EXPECT_EQ(FileUtils::getGameFileImageType("game.pbp"), IMAGE_PBP);
}

TEST_F(GameSpecificTest, GetGameFileImageTypeReturnsIMG) {
    EXPECT_EQ(FileUtils::getGameFileImageType("game.img"), IMAGE_IMG);
}

TEST_F(GameSpecificTest, GetGameFileImageTypeReturnsCHD) {
    EXPECT_EQ(FileUtils::getGameFileImageType("game.chd"), IMAGE_CHD);
}

TEST_F(GameSpecificTest, GetGameFileImageTypeReturnsNoGameFound) {
    EXPECT_EQ(FileUtils::getGameFileImageType("readme.txt"), IMAGE_NO_GAME_FOUND);
}

TEST_F(GameSpecificTest, ImageTypeUsesACueFileReturnsTrueForBin) {
    EXPECT_TRUE(FileUtils::imageTypeUsesACueFile(IMAGE_BIN));
}

TEST_F(GameSpecificTest, ImageTypeUsesACueFileReturnsTrueForIMG) {
    EXPECT_TRUE(FileUtils::imageTypeUsesACueFile(IMAGE_IMG));
}

TEST_F(GameSpecificTest, ImageTypeUsesACueFileReturnsFalseForPBP) {
    EXPECT_FALSE(FileUtils::imageTypeUsesACueFile(IMAGE_PBP));
}

TEST_F(GameSpecificTest, ImageTypeUsesACueFileReturnsFalseForCHD) {
    EXPECT_FALSE(FileUtils::imageTypeUsesACueFile(IMAGE_CHD));
}

TEST_F(GameSpecificTest, ThereIsAGameFileReturnsTrueWithGameFile) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/game.bin", content, true);

    EXPECT_TRUE(FileUtils::thereIsAGameFile(tempBase));
}

TEST_F(GameSpecificTest, ThereIsAGameFileReturnsFalseWithoutGameFile) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/readme.txt", content, true);

    EXPECT_FALSE(FileUtils::thereIsAGameFile(tempBase));
}

TEST_F(GameSpecificTest, ThereIsASubDirReturnsTrueWithSubdir) {
    FileUtils::createDir(tempBase + "/subdir");

    EXPECT_TRUE(FileUtils::thereIsASubDir(tempBase));
}

TEST_F(GameSpecificTest, ThereIsASubDirReturnsFalseWithoutSubdir) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file.txt", content, true);

    EXPECT_FALSE(FileUtils::thereIsASubDir(tempBase));
}

TEST_F(GameSpecificTest, GetGameFileReturnsGameFileInfo) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/game.pbp", content, true);
    FileUtils::writeTextFile(tempBase + "/readme.txt", content, true);

    auto result = FileUtils::getGameFile(tempBase);
    EXPECT_EQ(std::get<0>(result), IMAGE_PBP);
    EXPECT_EQ(std::get<1>(result), "game.pbp");
}

TEST_F(GameSpecificTest, GetGameFileReturnsNoGameFoundWhenEmpty) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/readme.txt", content, true);

    auto result = FileUtils::getGameFile(tempBase);
    EXPECT_EQ(std::get<0>(result), IMAGE_NO_GAME_FOUND);
    EXPECT_EQ(std::get<1>(result), "");
}

// ========================================
// Tests for Utility Functions
// ========================================

class UtilityFunctionsTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override {
        tempBase = "/tmp/util_test_" + std::to_string(rand());
        FileUtils::createDir(tempBase);
    }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(UtilityFunctionsTest, ReplaceTheseCharsWithThisCharReplacesMultiple) {
    std::string result = FileUtils::replaceTheseCharsWithThisChar("a,b;c:d", ",;:", '_');
    EXPECT_EQ(result, "a_b_c_d");
}

TEST_F(UtilityFunctionsTest, ReplaceTheseCharsWithThisCharHandlesNoMatch) {
    std::string result = FileUtils::replaceTheseCharsWithThisChar("abcd", "xyz", '_');
    EXPECT_EQ(result, "abcd");
}

TEST_F(UtilityFunctionsTest, ReplaceTheseCharsWithThisCharHandlesEmpty) {
    std::string result = FileUtils::replaceTheseCharsWithThisChar("", ",;:", '_');
    EXPECT_EQ(result, "");
}

TEST_F(UtilityFunctionsTest, FixCommaInDirOrFileNameReplacesComma) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file,name.txt", content, true);

    DirEntry entry("file,name.txt", false);
    bool changed = FileUtils::fixCommaInDirOrFileName(tempBase, &entry);

    EXPECT_TRUE(changed);
    EXPECT_EQ(entry.name, "file-name.txt");
    EXPECT_TRUE(FileUtils::exists(tempBase + "/file-name.txt"));
    EXPECT_FALSE(FileUtils::exists(tempBase + "/file,name.txt"));
}

TEST_F(UtilityFunctionsTest, FixCommaInDirOrFileNameReturnsFalseNoComma) {
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/filename.txt", content, true);

    DirEntry entry("filename.txt", false);
    bool changed = FileUtils::fixCommaInDirOrFileName(tempBase, &entry);

    EXPECT_FALSE(changed);
    EXPECT_EQ(entry.name, "filename.txt");
}

// ========================================
// Tests for DirEntry
// ========================================

class DirEntryTest : public ::testing::Test {};

TEST_F(DirEntryTest, ConstructorSetsFields) {
    DirEntry entry("test.txt", false);
    EXPECT_EQ(entry.name, "test.txt");
    EXPECT_FALSE(entry.isDir);

    DirEntry dirEntry("mydir", true);
    EXPECT_EQ(dirEntry.name, "mydir");
    EXPECT_TRUE(dirEntry.isDir);
}

TEST_F(DirEntryTest, SortByNameSortsCaseInsensitive) {
    DirEntry a("Apple", false);
    DirEntry b("banana", false);
    DirEntry c("Cherry", false);

    std::vector<DirEntry> entries = {c, a, b};
    std::sort(entries.begin(), entries.end(), DirEntry::sortByName);

    EXPECT_EQ(entries[0].name, "Apple");
    EXPECT_EQ(entries[1].name, "banana");
    EXPECT_EQ(entries[2].name, "Cherry");
}

// ========================================
// Tests for Separator Helper
// ========================================

class SeparatorTest : public ::testing::Test {};

TEST_F(SeparatorTest, SepAddsSeparatorWhenNeeded) {
    std::string path = "/home/user";
    path += sep;
    EXPECT_EQ(path, "/home/user/");
}

TEST_F(SeparatorTest, SepDoesNotAddDuplicateSeparator) {
    std::string path = "/home/user/";
    path += sep;
    EXPECT_EQ(path, "/home/user/");
}

TEST_F(SeparatorTest, SepHandlesEmptyString) {
    std::string path = "";
    path += sep;
    EXPECT_EQ(path, "");
}

TEST_F(SeparatorTest, SepConcatenationOperator) {
    std::string path = "/home/user";
    std::string result = path + sep;
    EXPECT_EQ(result, "/home/user/");
}

TEST_F(SeparatorTest, SepConcatenationWithFilename) {
    std::string dir = "/home/user";
    std::string filename = "file.txt";
    std::string result = dir + sep + filename;
    EXPECT_EQ(result, "/home/user/file.txt");
}

// ========================================
// Tests for matchExtension with various extension lengths
// ========================================

class MatchExtensionEdgeCasesTest : public ::testing::Test {};

TEST_F(MatchExtensionEdgeCasesTest, MatchesShortExtensions) {
    EXPECT_TRUE(FileUtils::matchExtension("file.7z", "7z"));
    EXPECT_TRUE(FileUtils::matchExtension("file.gz", ".gz"));
}

TEST_F(MatchExtensionEdgeCasesTest, MatchesLongExtensions) {
    EXPECT_TRUE(FileUtils::matchExtension("photo.jpeg", "jpeg"));
    EXPECT_TRUE(FileUtils::matchExtension("playlist.m3u8", "m3u8"));
    EXPECT_TRUE(FileUtils::matchExtension("archive.tar.gz", "gz"));
}

TEST_F(MatchExtensionEdgeCasesTest, MatchesSingleCharExtension) {
    EXPECT_TRUE(FileUtils::matchExtension("file.c", "c"));
    EXPECT_TRUE(FileUtils::matchExtension("file.h", ".h"));
}

TEST_F(MatchExtensionEdgeCasesTest, RejectsNoExtension) {
    EXPECT_FALSE(FileUtils::matchExtension("filename", "txt"));
    EXPECT_FALSE(FileUtils::matchExtension("Makefile", "txt"));
}

// ========================================
// Tests for cueToBinList
// ========================================

class CueToBinListTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override {
        tempBase = "/tmp/cue_test_" + std::to_string(rand());
        FileUtils::createDir(tempBase);
    }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(CueToBinListTest, ParsesSingleTrackCue) {
    std::string cueFile = tempBase + "/game.cue";
    std::ofstream out(cueFile);
    out << "FILE \"game.bin\" BINARY\n";
    out << "  TRACK 01 MODE2/2352\n";
    out << "    INDEX 01 00:00:00\n";
    out.close();

    auto binList = FileUtils::cueToBinList(cueFile);
    EXPECT_EQ(binList.size(), 1);
    EXPECT_EQ(binList[0], "game.bin");
}

TEST_F(CueToBinListTest, ParsesMultiTrackCue) {
    std::string cueFile = tempBase + "/multidisc.cue";
    std::ofstream out(cueFile);
    out << "FILE \"disc1.bin\" BINARY\n";
    out << "  TRACK 01 MODE2/2352\n";
    out << "    INDEX 01 00:00:00\n";
    out << "FILE \"disc2.bin\" BINARY\n";
    out << "  TRACK 02 AUDIO\n";
    out << "    INDEX 00 00:00:00\n";
    out << "FILE \"disc3.bin\" BINARY\n";
    out << "  TRACK 03 MODE2/2352\n";
    out << "    INDEX 01 00:00:00\n";
    out.close();

    auto binList = FileUtils::cueToBinList(cueFile);
    EXPECT_EQ(binList.size(), 3);
    EXPECT_EQ(binList[0], "disc1.bin");
    EXPECT_EQ(binList[1], "disc2.bin");
    EXPECT_EQ(binList[2], "disc3.bin");
}

TEST_F(CueToBinListTest, ReturnsEmptyForNonexistentFile) {
    auto binList = FileUtils::cueToBinList("/nonexistent/path.cue");
    EXPECT_EQ(binList.size(), 0);
}

TEST_F(CueToBinListTest, ReturnsEmptyForCueWithNoFiles) {
    std::string cueFile = tempBase + "/empty.cue";
    std::ofstream out(cueFile);
    out << "REM This is a comment\n";
    out << "TITLE \"Test\"\n";
    out.close();

    auto binList = FileUtils::cueToBinList(cueFile);
    EXPECT_EQ(binList.size(), 0);
}

// ========================================
// Tests for rmDir
// ========================================

class RmDirTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override { tempBase = "/tmp/rmdir_test_" + std::to_string(rand()); }

    void TearDown() override {
        // Clean up if test failed
        FileUtils::removeDirAndContents(tempBase);
    }
};

TEST_F(RmDirTest, RemovesEmptyDirectory) {
    FileUtils::createDir(tempBase);
    EXPECT_TRUE(FileUtils::exists(tempBase));

    int result = FileUtils::rmDir(tempBase);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(FileUtils::exists(tempBase));
}

TEST_F(RmDirTest, RemovesDirectoryWithFiles) {
    FileUtils::createDir(tempBase);
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file1.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/file2.txt", content, true);

    int result = FileUtils::rmDir(tempBase);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(FileUtils::exists(tempBase));
}

TEST_F(RmDirTest, RemovesNestedDirectories) {
    FileUtils::createDirRecursive(tempBase + "/level1/level2");
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/level1/file.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/level1/level2/deep.txt", content, true);

    int result = FileUtils::rmDir(tempBase);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(FileUtils::exists(tempBase));
}

TEST_F(RmDirTest, ReturnsErrorForNonexistent) {
    int result = FileUtils::rmDir("/nonexistent/path/12345");
    EXPECT_NE(result, 0);
}

// ========================================
// Tests for removeDirAndContents
// ========================================

class RemoveDirAndContentsTest : public ::testing::Test {
  protected:
    std::string tempBase;

    void SetUp() override { tempBase = "/tmp/removedir_test_" + std::to_string(rand()); }

    void TearDown() override { FileUtils::removeDirAndContents(tempBase); }
};

TEST_F(RemoveDirAndContentsTest, RemovesEmptyDirectory) {
    FileUtils::createDir(tempBase);
    EXPECT_TRUE(FileUtils::exists(tempBase));

    bool result = FileUtils::removeDirAndContents(tempBase);
    EXPECT_TRUE(result);
    EXPECT_FALSE(FileUtils::exists(tempBase));
}

TEST_F(RemoveDirAndContentsTest, RemovesDirectoryWithContents) {
    FileUtils::createDir(tempBase);
    FileUtils::createDir(tempBase + "/subdir");
    std::vector<std::string> content = {"test"};
    FileUtils::writeTextFile(tempBase + "/file.txt", content, true);
    FileUtils::writeTextFile(tempBase + "/subdir/nested.txt", content, true);

    bool result = FileUtils::removeDirAndContents(tempBase);
    EXPECT_TRUE(result);
    EXPECT_FALSE(FileUtils::exists(tempBase));
}

// ========================================
// Tests for Path Delegation Functions
// ========================================

class PathDelegationTest : public ::testing::Test {};

TEST_F(PathDelegationTest, FixPathTrimsAndRemovesTrailingSeparator) {
    EXPECT_EQ(FileUtils::fixPath("  /path/to/dir/  "), "/path/to/dir");
    EXPECT_EQ(FileUtils::fixPath("/path/to/dir/"), "/path/to/dir");
}

TEST_F(PathDelegationTest, RemoveSeparatorFromEndOfPath) {
    EXPECT_EQ(FileUtils::removeSeparatorFromEndOfPath("/path/"), "/path");
    EXPECT_EQ(FileUtils::removeSeparatorFromEndOfPath("/path"), "/path");
}

TEST_F(PathDelegationTest, GetFileNameFromPath) {
    EXPECT_EQ(FileUtils::getFileNameFromPath("/path/to/file.txt"), "file.txt");
    EXPECT_EQ(FileUtils::getFileNameFromPath("file.txt"), "file.txt");
}

TEST_F(PathDelegationTest, GetDirNameFromPath) {
    EXPECT_EQ(FileUtils::getDirNameFromPath("/path/to/file.txt"), "/path/to");
}

TEST_F(PathDelegationTest, GetFileExtension) {
    // Note: getFileExtension returns extension WITHOUT the dot
    EXPECT_EQ(FileUtils::getFileExtension("file.txt"), "txt");
    EXPECT_EQ(FileUtils::getFileExtension("file.tar.gz"), "gz");
}

TEST_F(PathDelegationTest, GetFileNameWithoutExtension) {
    EXPECT_EQ(FileUtils::getFileNameWithoutExtension("file.txt"), "file");
    // Note: Works on full path, returns full path without extension
    EXPECT_EQ(FileUtils::getFileNameWithoutExtension("/path/to/file.txt"), "/path/to/file");
}
