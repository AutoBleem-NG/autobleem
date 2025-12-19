// Unit tests for Inifile class
//
// Tests INI file parsing, saving, and key-value lookup operations.
// Creates temporary files that are cleaned up after each test.

#include "engine/ini_file.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

class InifileTest : public ::testing::Test {
  protected:
    std::string testFilePath;

    void SetUp() override {
        testFilePath = "/tmp/test_inifile_" + std::to_string(rand()) + ".ini";
    }

    void TearDown() override {
        std::remove(testFilePath.c_str());
    }

    void writeTestFile(const std::string &content) {
        std::ofstream os(testFilePath);
        os << content;
        os.close();
    }
};

//=============================================================================
// Load Tests
//=============================================================================

TEST_F(InifileTest, LoadSimpleFile) {
    writeTestFile("[Game]\n"
                  "Title=Test Game\n"
                  "Year=1999\n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.section, "Game");
    EXPECT_EQ(ini.values["title"], "Test Game");
    EXPECT_EQ(ini.values["year"], "1999");
}

TEST_F(InifileTest, LoadFileWithComments) {
    writeTestFile("[Game]\n"
                  "Title=Test Game # inline comment\n"
                  "# full line comment\n"
                  "Year=2000\n");

    Inifile ini;
    ini.load(testFilePath);

    // Comment is removed, and the line is trimmed (removing trailing space before #)
    EXPECT_EQ(ini.values["title"], "Test Game");
    EXPECT_EQ(ini.values["year"], "2000");
}

TEST_F(InifileTest, LoadFileWithBlankLines) {
    writeTestFile("[Game]\n"
                  "\n"
                  "Title=Test\n"
                  "\n"
                  "Year=2001\n"
                  "\n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.values.size(), 2);
    EXPECT_EQ(ini.values["title"], "Test");
    EXPECT_EQ(ini.values["year"], "2001");
}

TEST_F(InifileTest, LoadFileWithWindowsLineEndings) {
    writeTestFile("[Game]\r\n"
                  "Title=Test\r\n"
                  "Year=2002\r\n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.values["title"], "Test");
    EXPECT_EQ(ini.values["year"], "2002");
}

TEST_F(InifileTest, KeysAreLowercase) {
    writeTestFile("[Game]\n"
                  "TITLE=Test\n"
                  "YeAr=1998\n");

    Inifile ini;
    ini.load(testFilePath);

    // Keys should be stored lowercase
    EXPECT_TRUE(ini.values.find("title") != ini.values.end());
    EXPECT_TRUE(ini.values.find("year") != ini.values.end());
    EXPECT_TRUE(ini.values.find("TITLE") == ini.values.end());
}

TEST_F(InifileTest, LoadNonexistentFile) {
    Inifile ini;
    ini.load("/nonexistent/path/file.ini");

    // Should handle gracefully without crashing
    EXPECT_TRUE(ini.values.empty());
}

TEST_F(InifileTest, LoadEmptyFile) {
    writeTestFile("");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_TRUE(ini.values.empty());
}

TEST_F(InifileTest, LoadFileWithPublisher) {
    writeTestFile("[Game]\n"
                  "Publisher=Sony Computer Entertainment.\n");

    Inifile ini;
    ini.load(testFilePath);

    // Publisher should be cleaned (trailing period removed)
    EXPECT_EQ(ini.values["publisher"], "Sony Computer Entertainment");
}

//=============================================================================
// Reload Tests
//=============================================================================

TEST_F(InifileTest, ReloadClearsPreviousValues) {
    writeTestFile("[Game]\n"
                  "Title=First\n"
                  "Extra=Value\n");

    Inifile ini;
    ini.load(testFilePath);
    EXPECT_EQ(ini.values["title"], "First");
    EXPECT_EQ(ini.values["extra"], "Value");

    // Rewrite file with different content
    writeTestFile("[Game]\n"
                  "Title=Second\n");

    ini.reload(testFilePath);
    EXPECT_EQ(ini.values["title"], "Second");
    // Extra should be gone after reload
    EXPECT_TRUE(ini.values.find("extra") == ini.values.end());
}

//=============================================================================
// Get Tests
//=============================================================================

TEST_F(InifileTest, GetExistingKey) {
    writeTestFile("[Game]\n"
                  "title=MyGame\n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.get("title"), "MyGame");
    EXPECT_EQ(ini.get("title", "Default"), "MyGame");
}

TEST_F(InifileTest, GetMissingKeyReturnsDefault) {
    writeTestFile("[Game]\n"
                  "title=MyGame\n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.get("nonexistent"), "");
    EXPECT_EQ(ini.get("nonexistent", "DefaultValue"), "DefaultValue");
}

TEST_F(InifileTest, GetEmptyValueReturnsDefault) {
    writeTestFile("[Game]\n"
                  "title=\n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.get("title", "DefaultTitle"), "DefaultTitle");
}

TEST_F(InifileTest, GetTrimsWhitespace) {
    writeTestFile("[Game]\n"
                  "title=  Trimmed  \n");

    Inifile ini;
    ini.load(testFilePath);

    EXPECT_EQ(ini.get("title"), "Trimmed");
}

//=============================================================================
// Save Tests
//=============================================================================

TEST_F(InifileTest, SaveCreatesFile) {
    Inifile ini;
    ini.section = "TestSection";
    ini.values["key1"] = "value1";
    ini.values["key2"] = "value2";

    ini.save(testFilePath);

    // Read back the file
    Inifile loaded;
    loaded.load(testFilePath);

    EXPECT_EQ(loaded.section, "TestSection");
    EXPECT_EQ(loaded.values["key1"], "value1");
    EXPECT_EQ(loaded.values["key2"], "value2");
}

TEST_F(InifileTest, SaveCapitalizesFirstLetter) {
    Inifile ini;
    ini.section = "Game";
    ini.values["title"] = "MyGame";

    ini.save(testFilePath);

    // Read raw file to check formatting
    std::ifstream is(testFilePath);
    std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    is.close();

    EXPECT_TRUE(content.find("Title=") != std::string::npos);
}

//=============================================================================
// OverwriteAndAppend Tests
//=============================================================================

TEST_F(InifileTest, OverwriteAndAppendMergesValues) {
    Inifile ini;
    ini.values["existing"] = "original";
    ini.values["tooverwrite"] = "old";

    writeTestFile("[Game]\n"
                  "tooverwrite=new\n"
                  "newkey=added\n");

    ini.OverwriteAndAppend(testFilePath);

    EXPECT_EQ(ini.values["existing"], "original");
    EXPECT_EQ(ini.values["tooverwrite"], "new");
    EXPECT_EQ(ini.values["newkey"], "added");
}

//=============================================================================
// Edge Cases
//=============================================================================

TEST_F(InifileTest, ValueWithEqualsSign) {
    writeTestFile("[Game]\n"
                  "formula=a=b+c\n");

    Inifile ini;
    ini.load(testFilePath);

    // Value should include everything after first '='
    EXPECT_EQ(ini.values["formula"], "a=b+c");
}

TEST_F(InifileTest, SectionWithoutBracketEnd) {
    writeTestFile("[Game\n"
                  "Title=Test\n");

    Inifile ini;
    ini.load(testFilePath);

    // Should handle gracefully
    EXPECT_EQ(ini.values["title"], "Test");
}

TEST_F(InifileTest, MultipleSectons) {
    writeTestFile("[First]\n"
                  "Key1=Val1\n"
                  "[Second]\n"
                  "Key2=Val2\n");

    Inifile ini;
    ini.load(testFilePath);

    // Current implementation only tracks last section name
    EXPECT_EQ(ini.section, "Second");
    // But all values are merged
    EXPECT_EQ(ini.values["key1"], "Val1");
    EXPECT_EQ(ini.values["key2"], "Val2");
}
