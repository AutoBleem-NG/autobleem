// Unit tests for string utilities
#include <gtest/gtest.h>
#include <vector>

#include "../code/utils/string_utils.h"

// Tests for ltrim (trim from start)
class LtrimTest : public ::testing::Test {};

TEST_F(LtrimTest, RemovesLeadingSpaces) {
    std::string s = "   hello";
    EXPECT_EQ(ltrim(s), "hello");
    EXPECT_EQ(s, "hello");
}

TEST_F(LtrimTest, RemovesLeadingTabs) {
    std::string s = "\t\thello";
    EXPECT_EQ(ltrim(s), "hello");
}

TEST_F(LtrimTest, RemovesMixedWhitespace) {
    std::string s = " \t \n hello";
    EXPECT_EQ(ltrim(s), "hello");
}

TEST_F(LtrimTest, PreservesTrailingWhitespace) {
    std::string s = "   hello   ";
    EXPECT_EQ(ltrim(s), "hello   ");
}

TEST_F(LtrimTest, HandlesNoLeadingWhitespace) {
    std::string s = "hello";
    EXPECT_EQ(ltrim(s), "hello");
}

TEST_F(LtrimTest, HandlesEmptyString) {
    std::string s = "";
    EXPECT_EQ(ltrim(s), "");
}

TEST_F(LtrimTest, HandlesAllWhitespace) {
    std::string s = "   ";
    EXPECT_EQ(ltrim(s), "");
}

// Tests for rtrim (trim from end)
class RtrimTest : public ::testing::Test {};

TEST_F(RtrimTest, RemovesTrailingSpaces) {
    std::string s = "hello   ";
    EXPECT_EQ(rtrim(s), "hello");
    EXPECT_EQ(s, "hello");
}

TEST_F(RtrimTest, RemovesTrailingTabs) {
    std::string s = "hello\t\t";
    EXPECT_EQ(rtrim(s), "hello");
}

TEST_F(RtrimTest, RemovesMixedWhitespace) {
    std::string s = "hello \t \n ";
    EXPECT_EQ(rtrim(s), "hello");
}

TEST_F(RtrimTest, PreservesLeadingWhitespace) {
    std::string s = "   hello   ";
    EXPECT_EQ(rtrim(s), "   hello");
}

TEST_F(RtrimTest, HandlesNoTrailingWhitespace) {
    std::string s = "hello";
    EXPECT_EQ(rtrim(s), "hello");
}

TEST_F(RtrimTest, HandlesEmptyString) {
    std::string s = "";
    EXPECT_EQ(rtrim(s), "");
}

TEST_F(RtrimTest, HandlesAllWhitespace) {
    std::string s = "   ";
    EXPECT_EQ(rtrim(s), "");
}

// Tests for trim (trim from both ends)
class TrimTest : public ::testing::Test {};

TEST_F(TrimTest, RemovesBothEnds) {
    std::string s = "   hello   ";
    EXPECT_EQ(trim(s), "hello");
    EXPECT_EQ(s, "hello");
}

TEST_F(TrimTest, PreservesInternalWhitespace) {
    std::string s = "   hello world   ";
    EXPECT_EQ(trim(s), "hello world");
}

TEST_F(TrimTest, HandlesNoWhitespace) {
    std::string s = "hello";
    EXPECT_EQ(trim(s), "hello");
}

TEST_F(TrimTest, HandlesEmptyString) {
    std::string s = "";
    EXPECT_EQ(trim(s), "");
}

// Tests for lcase (in-place lowercase)
class LcaseTest : public ::testing::Test {};

TEST_F(LcaseTest, ConvertsAllToLowercase) {
    std::string s = "HELLO WORLD";
    EXPECT_EQ(lcase(s), "hello world");
    EXPECT_EQ(s, "hello world");
}

TEST_F(LcaseTest, ConvertsPartialString) {
    std::string s = "HELLO WORLD";
    EXPECT_EQ(lcase(s, 5), "hello WORLD");
}

TEST_F(LcaseTest, HandlesMixedCase) {
    std::string s = "HeLLo WoRLd";
    EXPECT_EQ(lcase(s), "hello world");
}

TEST_F(LcaseTest, HandlesAlreadyLowercase) {
    std::string s = "hello";
    EXPECT_EQ(lcase(s), "hello");
}

TEST_F(LcaseTest, PreservesNumbers) {
    std::string s = "ABC123XYZ";
    EXPECT_EQ(lcase(s), "abc123xyz");
}

TEST_F(LcaseTest, HandlesEmptyString) {
    std::string s = "";
    EXPECT_EQ(lcase(s), "");
}

TEST_F(LcaseTest, ZeroNcharsReturnsFullLowercase) {
    std::string s = "HELLO";
    EXPECT_EQ(lcase(s, 0), "hello");
}

// Tests for ucase (in-place uppercase)
class UcaseTest : public ::testing::Test {};

TEST_F(UcaseTest, ConvertsAllToUppercase) {
    std::string s = "hello world";
    EXPECT_EQ(ucase(s), "HELLO WORLD");
    EXPECT_EQ(s, "HELLO WORLD");
}

TEST_F(UcaseTest, ConvertsPartialString) {
    std::string s = "hello world";
    EXPECT_EQ(ucase(s, 5), "HELLO world");
}

TEST_F(UcaseTest, HandlesMixedCase) {
    std::string s = "HeLLo WoRLd";
    EXPECT_EQ(ucase(s), "HELLO WORLD");
}

TEST_F(UcaseTest, HandlesAlreadyUppercase) {
    std::string s = "HELLO";
    EXPECT_EQ(ucase(s), "HELLO");
}

TEST_F(UcaseTest, PreservesNumbers) {
    std::string s = "abc123xyz";
    EXPECT_EQ(ucase(s), "ABC123XYZ");
}

TEST_F(UcaseTest, HandlesEmptyString) {
    std::string s = "";
    EXPECT_EQ(ucase(s), "");
}

// Tests for ReturnLowerCase (returns copy)
class ReturnLowerCaseTest : public ::testing::Test {};

TEST_F(ReturnLowerCaseTest, ReturnsLowercaseCopy) {
    const std::string s = "HELLO WORLD";
    EXPECT_EQ(ReturnLowerCase(s), "hello world");
    EXPECT_EQ(s, "HELLO WORLD");  // Original unchanged
}

TEST_F(ReturnLowerCaseTest, HandlesMixedCase) {
    EXPECT_EQ(ReturnLowerCase("HeLLo WoRLd"), "hello world");
}

TEST_F(ReturnLowerCaseTest, HandlesEmptyString) {
    EXPECT_EQ(ReturnLowerCase(""), "");
}

TEST_F(ReturnLowerCaseTest, PreservesSpecialCharacters) {
    EXPECT_EQ(ReturnLowerCase("ABC!@#123"), "abc!@#123");
}

// Tests for ReturnUpperCase (returns copy)
class ReturnUpperCaseTest : public ::testing::Test {};

TEST_F(ReturnUpperCaseTest, ReturnsUppercaseCopy) {
    const std::string s = "hello world";
    EXPECT_EQ(ReturnUpperCase(s), "HELLO WORLD");
    EXPECT_EQ(s, "hello world");  // Original unchanged
}

TEST_F(ReturnUpperCaseTest, HandlesMixedCase) {
    EXPECT_EQ(ReturnUpperCase("HeLLo WoRLd"), "HELLO WORLD");
}

TEST_F(ReturnUpperCaseTest, HandlesEmptyString) {
    EXPECT_EQ(ReturnUpperCase(""), "");
}

TEST_F(ReturnUpperCaseTest, PreservesSpecialCharacters) {
    EXPECT_EQ(ReturnUpperCase("abc!@#123"), "ABC!@#123");
}

// Tests for SortByCaseInsensitive
class SortByCaseInsensitiveTest : public ::testing::Test {};

TEST_F(SortByCaseInsensitiveTest, ComparesIgnoringCase) {
    EXPECT_TRUE(SortByCaseInsensitive("apple", "Banana"));
    EXPECT_FALSE(SortByCaseInsensitive("Banana", "apple"));
}

TEST_F(SortByCaseInsensitiveTest, SameCaseDifferentValues) {
    EXPECT_TRUE(SortByCaseInsensitive("aaa", "bbb"));
    EXPECT_FALSE(SortByCaseInsensitive("bbb", "aaa"));
}

TEST_F(SortByCaseInsensitiveTest, EqualStrings) {
    EXPECT_FALSE(SortByCaseInsensitive("Hello", "hello"));
    EXPECT_FALSE(SortByCaseInsensitive("hello", "Hello"));
}

TEST_F(SortByCaseInsensitiveTest, EmptyStrings) {
    EXPECT_FALSE(SortByCaseInsensitive("", ""));
    EXPECT_TRUE(SortByCaseInsensitive("", "a"));
    EXPECT_FALSE(SortByCaseInsensitive("a", ""));
}

TEST_F(SortByCaseInsensitiveTest, WorksWithStdSort) {
    std::vector<std::string> games = {"Zelda", "mario", "DONKEY KONG", "link"};
    std::sort(games.begin(), games.end(), SortByCaseInsensitive);

    EXPECT_EQ(games[0], "DONKEY KONG");
    EXPECT_EQ(games[1], "link");
    EXPECT_EQ(games[2], "mario");
    EXPECT_EQ(games[3], "Zelda");
}
