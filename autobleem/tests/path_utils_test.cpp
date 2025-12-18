#include <gtest/gtest.h>

#include "../code/utils/path_utils.h"

class GetFileExtensionTest : public ::testing::Test {};

TEST_F(GetFileExtensionTest, ExtractsSimpleExtension) {
    std::string result = PathUtils::getFileExtension("file.txt");
    EXPECT_EQ(result, "txt");
}

TEST_F(GetFileExtensionTest, ExtractsLowerCaseExtension) {
    std::string result = PathUtils::getFileExtension("document.PDF");
    EXPECT_EQ(result, "PDF");
}

TEST_F(GetFileExtensionTest, HandlesMultipleDots) {
    std::string result = PathUtils::getFileExtension("file.tar.gz");
    EXPECT_EQ(result, "gz");
}

TEST_F(GetFileExtensionTest, HandlesNoExtension) {
    std::string result = PathUtils::getFileExtension("README");
    EXPECT_EQ(result, "");
}

TEST_F(GetFileExtensionTest, HandlesHiddenFile) {
    std::string result = PathUtils::getFileExtension(".bashrc");
    EXPECT_EQ(result, "bashrc");
}

TEST_F(GetFileExtensionTest, HandlesEmptyString) {
    std::string result = PathUtils::getFileExtension("");
    EXPECT_EQ(result, "");
}

TEST_F(GetFileExtensionTest, HandlesPathWithExtension) {
    std::string result = PathUtils::getFileExtension("/path/to/file.cpp");
    EXPECT_EQ(result, "cpp");
}

// Tests for getFileNameWithoutExtension
class GetFileNameWithoutExtensionTest : public ::testing::Test {};

TEST_F(GetFileNameWithoutExtensionTest, RemovesSimpleExtension) {
    std::string result = PathUtils::getFileNameWithoutExtension("document.txt");
    EXPECT_EQ(result, "document");
}

TEST_F(GetFileNameWithoutExtensionTest, HandlesNoExtension) {
    std::string result = PathUtils::getFileNameWithoutExtension("README");
    EXPECT_EQ(result, "README");
}

TEST_F(GetFileNameWithoutExtensionTest, HandlesMultipleDots) {
    std::string result = PathUtils::getFileNameWithoutExtension("archive.tar.gz");
    EXPECT_EQ(result, "archive.tar");
}

TEST_F(GetFileNameWithoutExtensionTest, HandlesPathWithExtension) {
    std::string result = PathUtils::getFileNameWithoutExtension("/path/to/file.cpp");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(GetFileNameWithoutExtensionTest, HandlesDotAtStart) {
    std::string result = PathUtils::getFileNameWithoutExtension(".bashrc");
    EXPECT_EQ(result, "");
}

// Tests for getFileNameFromPath
class GetFileNameFromPathTest : public ::testing::Test {};

TEST_F(GetFileNameFromPathTest, ExtractsSimpleFilename) {
    std::string result = PathUtils::getFileNameFromPath("/path/to/file.txt");
    EXPECT_EQ(result, "file.txt");
}

TEST_F(GetFileNameFromPathTest, HandlesSingleComponent) {
    std::string result = PathUtils::getFileNameFromPath("file.txt");
    EXPECT_EQ(result, "file.txt");
}

TEST_F(GetFileNameFromPathTest, HandlesTrailingSeparator) {
    std::string result = PathUtils::getFileNameFromPath("/path/to/");
    // Result depends on basename() behavior with trailing separator
    EXPECT_FALSE(result.empty());
}

TEST_F(GetFileNameFromPathTest, HandlesRootPath) {
    std::string result = PathUtils::getFileNameFromPath("/");
    EXPECT_EQ(result, "/");
}

// Tests for getDirNameFromPath
class GetDirNameFromPathTest : public ::testing::Test {};

TEST_F(GetDirNameFromPathTest, ExtractsDirFromPath) {
    std::string result = PathUtils::getDirNameFromPath("/path/to/file.txt");
    EXPECT_EQ(result, "/path/to");
}

TEST_F(GetDirNameFromPathTest, HandlesSingleComponent) {
    std::string result = PathUtils::getDirNameFromPath("file.txt");
    EXPECT_EQ(result, ".");
}

TEST_F(GetDirNameFromPathTest, HandlesRootPath) {
    std::string result = PathUtils::getDirNameFromPath("/");
    EXPECT_EQ(result, "/");
}

TEST_F(GetDirNameFromPathTest, HandlesTrailingSeparator) {
    std::string result = PathUtils::getDirNameFromPath("/path/to/");
    // dirname() removes trailing separator, so /path/to/ becomes /path
    EXPECT_EQ(result, "/path");
}

// Tests for joinPath
class JoinPathTest : public ::testing::Test {};

TEST_F(JoinPathTest, JoinsTwoPaths) {
    std::string result = PathUtils::joinPath("/path/to", "file.txt");
    EXPECT_EQ(result, "/path/to/file.txt");
}

TEST_F(JoinPathTest, AvoidsDuplicateSeparator) {
    std::string result = PathUtils::joinPath("/path/to/", "file.txt");
    EXPECT_EQ(result, "/path/to/file.txt");
}

TEST_F(JoinPathTest, HandlesEmptyDir) {
    std::string result = PathUtils::joinPath("", "file.txt");
    EXPECT_EQ(result, "file.txt");
}

TEST_F(JoinPathTest, HandlesEmptyFile) {
    std::string result = PathUtils::joinPath("/path/to", "");
    EXPECT_EQ(result, "/path/to");
}

TEST_F(JoinPathTest, HandlesBothEmpty) {
    std::string result = PathUtils::joinPath("", "");
    EXPECT_EQ(result, "");
}

TEST_F(JoinPathTest, HandlesRelativePath) {
    std::string result = PathUtils::joinPath("path/to", "file.txt");
    EXPECT_EQ(result, "path/to/file.txt");
}

// Tests for removeSeparatorFromEndOfPath
class RemoveSeparatorFromEndOfPathTest : public ::testing::Test {};

TEST_F(RemoveSeparatorFromEndOfPathTest, RemovesTrailingSeparator) {
    std::string result = PathUtils::removeSeparatorFromEndOfPath("/path/to/");
    EXPECT_EQ(result, "/path/to");
}

TEST_F(RemoveSeparatorFromEndOfPathTest, HandlesNoTrailingSeparator) {
    std::string result = PathUtils::removeSeparatorFromEndOfPath("/path/to");
    EXPECT_EQ(result, "/path/to");
}

TEST_F(RemoveSeparatorFromEndOfPathTest, HandlesSingleSlash) {
    std::string result = PathUtils::removeSeparatorFromEndOfPath("/");
    EXPECT_EQ(result, "");
}

TEST_F(RemoveSeparatorFromEndOfPathTest, HandlesEmptyString) {
    std::string result = PathUtils::removeSeparatorFromEndOfPath("");
    EXPECT_EQ(result, "");
}

// Tests for fixPath
class FixPathTest : public ::testing::Test {};

TEST_F(FixPathTest, RemovesLeadingSpaces) {
    std::string result = PathUtils::fixPath("   /path/to/file");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(FixPathTest, RemovesTrailingSpaces) {
    std::string result = PathUtils::fixPath("/path/to/file   ");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(FixPathTest, RemovesBothLeadingAndTrailingSpaces) {
    std::string result = PathUtils::fixPath("   /path/to/file   ");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(FixPathTest, RemovesTrailingSeparator) {
    std::string result = PathUtils::fixPath("/path/to/file/");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(FixPathTest, RemovesSpacesAndSeparator) {
    std::string result = PathUtils::fixPath("   /path/to/file/   ");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(FixPathTest, HandlesEmptyString) {
    std::string result = PathUtils::fixPath("");
    EXPECT_EQ(result, "");
}

TEST_F(FixPathTest, HandlesOnlySpaces) {
    std::string result = PathUtils::fixPath("   ");
    EXPECT_EQ(result, "");
}

TEST_F(FixPathTest, HandlesOnlyNewlines) {
    std::string result = PathUtils::fixPath("\n\n");
    EXPECT_EQ(result, "");
}

TEST_F(FixPathTest, PreservesInternalSpaces) {
    std::string result = PathUtils::fixPath("  /path with spaces/file  ");
    EXPECT_EQ(result, "/path with spaces/file");
}

TEST_F(FixPathTest, HandlesMultipleTrailingSeparators) {
    std::string result = PathUtils::fixPath("/path/to/file///");
    // Only removes trailing, not all trailing separators
    EXPECT_EQ(result, "/path/to/file//");
}

class SeparatorConstantTest : public ::testing::Test {};

TEST_F(SeparatorConstantTest, SeparatorIsForwardSlashOnUnix) {
#ifndef _WIN32
    EXPECT_EQ(PathUtils::separator, '/');
#endif
}

TEST_F(SeparatorConstantTest, SeparatorIsBackslashOnWindows) {
#ifdef _WIN32
    EXPECT_EQ(PathUtils::separator, '\\');
#endif
}

class PathUtilsEdgeCasesTest : public ::testing::Test {};

TEST_F(PathUtilsEdgeCasesTest, GetFileExtensionWithOnlyDot) {
    EXPECT_EQ(PathUtils::getFileExtension("file."), "");
}

TEST_F(PathUtilsEdgeCasesTest, GetFileExtensionWithDotsInPath) {
    EXPECT_EQ(PathUtils::getFileExtension("/path.with.dots/file.txt"), "txt");
}

TEST_F(PathUtilsEdgeCasesTest, GetFileNameWithoutExtensionOnlyDot) {
    EXPECT_EQ(PathUtils::getFileNameWithoutExtension("file."), "file");
}

TEST_F(PathUtilsEdgeCasesTest, JoinPathWithLeadingSlashInFile) {
    std::string result = PathUtils::joinPath("/path/to", "/file.txt");
    // File has leading separator - joins directly
    EXPECT_EQ(result, "/path/to//file.txt");
}

TEST_F(PathUtilsEdgeCasesTest, FixPathRemovesTabs) {
    std::string result = PathUtils::fixPath("\t/path/to/file\t");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(PathUtilsEdgeCasesTest, FixPathRemovesCarriageReturn) {
    std::string result = PathUtils::fixPath("\r/path/to/file\r");
    EXPECT_EQ(result, "/path/to/file");
}

TEST_F(PathUtilsEdgeCasesTest, GetDirNameFromPathWithMultipleLevels) {
    std::string result = PathUtils::getDirNameFromPath("/a/b/c/d/file.txt");
    EXPECT_EQ(result, "/a/b/c/d");
}

TEST_F(PathUtilsEdgeCasesTest, GetFileNameFromPathWithDots) {
    std::string result = PathUtils::getFileNameFromPath("/path/to/file.tar.gz");
    EXPECT_EQ(result, "file.tar.gz");
}

TEST_F(PathUtilsEdgeCasesTest, GetFileExtensionDirWithDotNoFileExtension) {
    // Known limitation: getFileExtension operates on the full path string,
    // not just the filename component. When a directory has a dot and the
    // file has no extension, it returns everything after the last dot.
    std::string result = PathUtils::getFileExtension("/path.dir/to/README");
    EXPECT_EQ(result, "dir/to/README");
}

TEST_F(PathUtilsEdgeCasesTest, GetFileExtensionDirWithDotFileHasExtension) {
    std::string result = PathUtils::getFileExtension("/path.dir/to/file.txt");
    EXPECT_EQ(result, "txt");
}

TEST_F(PathUtilsEdgeCasesTest, GetFileNameFromPathEmptyString) {
    std::string result = PathUtils::getFileNameFromPath("");
    EXPECT_FALSE(result.empty());  // basename("") returns "."
}

TEST_F(PathUtilsEdgeCasesTest, GetDirNameFromPathEmptyString) {
    std::string result = PathUtils::getDirNameFromPath("");
    EXPECT_FALSE(result.empty());  // dirname("") returns "."
}

TEST_F(PathUtilsEdgeCasesTest, JoinPathMultipleSeparatorsInFile) {
    std::string result = PathUtils::joinPath("/path", "subdir/file.txt");
    EXPECT_EQ(result, "/path/subdir/file.txt");
}

TEST_F(PathUtilsEdgeCasesTest, FixPathOnlySeparator) {
    std::string result = PathUtils::fixPath("/");
    EXPECT_EQ(result, "");
}

TEST_F(PathUtilsEdgeCasesTest, FixPathMixedWhitespace) {
    std::string result = PathUtils::fixPath(" \t\n/path/to/file\r\n ");
    EXPECT_EQ(result, "/path/to/file");
}
