// Unit tests for string utilities (both inline and StringUtils namespace functions)
#include <gtest/gtest.h>
#include <vector>

#include "../code/utils/string_utils.h"

// =============================================================================
// Tests for StringUtils namespace functions (implemented in string_utils.cpp)
// =============================================================================

// Tests for escape() and decode()
class EscapeDecodeTest : public ::testing::Test {};

TEST_F(EscapeDecodeTest, EscapesPipeCharacter) {
    EXPECT_EQ(StringUtils::escape("hello|world"), "hello||world");
}

TEST_F(EscapeDecodeTest, EscapesCommaCharacter) {
    EXPECT_EQ(StringUtils::escape("hello,world"), "hello|@world");
}

TEST_F(EscapeDecodeTest, EscapesBothCharacters) {
    EXPECT_EQ(StringUtils::escape("a|b,c"), "a||b|@c");
}

TEST_F(EscapeDecodeTest, EscapesMultiplePipes) {
    EXPECT_EQ(StringUtils::escape("||"), "||||");
}

TEST_F(EscapeDecodeTest, EscapesEmptyString) {
    EXPECT_EQ(StringUtils::escape(""), "");
}

TEST_F(EscapeDecodeTest, EscapesStringWithNoSpecialChars) {
    EXPECT_EQ(StringUtils::escape("hello world"), "hello world");
}

TEST_F(EscapeDecodeTest, DecodesPipeCharacter) {
    EXPECT_EQ(StringUtils::decode("hello||world"), "hello|world");
}

TEST_F(EscapeDecodeTest, DecodesCommaCharacter) {
    EXPECT_EQ(StringUtils::decode("hello|@world"), "hello,world");
}

TEST_F(EscapeDecodeTest, DecodesBothCharacters) {
    EXPECT_EQ(StringUtils::decode("a||b|@c"), "a|b,c");
}

TEST_F(EscapeDecodeTest, DecodesEmptyString) {
    EXPECT_EQ(StringUtils::decode(""), "");
}

TEST_F(EscapeDecodeTest, RoundTripPreservesString) {
    std::string original = "Game: Metal Gear | Solid, Publisher: Konami";
    std::string escaped = StringUtils::escape(original);
    std::string decoded = StringUtils::decode(escaped);
    EXPECT_EQ(decoded, original);
}

TEST_F(EscapeDecodeTest, RoundTripWithOnlySpecialChars) {
    std::string original = "|,|,|";
    std::string escaped = StringUtils::escape(original);
    std::string decoded = StringUtils::decode(escaped);
    EXPECT_EQ(decoded, original);
}

// Tests for replaceAll()
class ReplaceAllTest : public ::testing::Test {};

TEST_F(ReplaceAllTest, ReplacesSingleOccurrence) {
    std::string s = "hello world";
    StringUtils::replaceAll(s, "world", "there");
    EXPECT_EQ(s, "hello there");
}

TEST_F(ReplaceAllTest, ReplacesMultipleOccurrences) {
    std::string s = "aaa";
    StringUtils::replaceAll(s, "a", "b");
    EXPECT_EQ(s, "bbb");
}

TEST_F(ReplaceAllTest, HandlesEmptyFromString) {
    std::string s = "hello";
    StringUtils::replaceAll(s, "", "x");
    EXPECT_EQ(s, "hello");
}

TEST_F(ReplaceAllTest, ReplacesWithEmptyString) {
    std::string s = "hello world";
    StringUtils::replaceAll(s, " ", "");
    EXPECT_EQ(s, "helloworld");
}

TEST_F(ReplaceAllTest, ReplacesWithLongerString) {
    std::string s = "a-b-c";
    StringUtils::replaceAll(s, "-", "---");
    EXPECT_EQ(s, "a---b---c");
}

TEST_F(ReplaceAllTest, HandlesNoMatch) {
    std::string s = "hello";
    StringUtils::replaceAll(s, "x", "y");
    EXPECT_EQ(s, "hello");
}

TEST_F(ReplaceAllTest, HandlesToContainingFrom) {
    std::string s = "abc";
    StringUtils::replaceAll(s, "b", "xbx");
    EXPECT_EQ(s, "axbxc");
}

// Tests for isInteger()
class IsIntegerTest : public ::testing::Test {};

TEST_F(IsIntegerTest, ReturnsTrueForDigits) {
    EXPECT_TRUE(StringUtils::isInteger("123"));
    EXPECT_TRUE(StringUtils::isInteger("0"));
    EXPECT_TRUE(StringUtils::isInteger("9876543210"));
}

TEST_F(IsIntegerTest, ReturnsFalseForNullptr) {
    EXPECT_FALSE(StringUtils::isInteger(nullptr));
}

TEST_F(IsIntegerTest, ReturnsFalseForEmptyString) {
    EXPECT_FALSE(StringUtils::isInteger(""));
}

TEST_F(IsIntegerTest, ReturnsFalseForLetters) {
    EXPECT_FALSE(StringUtils::isInteger("abc"));
    EXPECT_FALSE(StringUtils::isInteger("123abc"));
    EXPECT_FALSE(StringUtils::isInteger("abc123"));
}

TEST_F(IsIntegerTest, ReturnsFalseForNegativeSign) {
    EXPECT_FALSE(StringUtils::isInteger("-123"));
}

TEST_F(IsIntegerTest, ReturnsFalseForDecimalPoint) {
    EXPECT_FALSE(StringUtils::isInteger("12.34"));
}

TEST_F(IsIntegerTest, ReturnsFalseForWhitespace) {
    EXPECT_FALSE(StringUtils::isInteger(" 123"));
    EXPECT_FALSE(StringUtils::isInteger("123 "));
}

// Tests for compareCaseInsensitive()
class CompareCaseInsensitiveTest : public ::testing::Test {};

TEST_F(CompareCaseInsensitiveTest, MatchesSameCase) {
    EXPECT_TRUE(StringUtils::compareCaseInsensitive("hello", "hello"));
}

TEST_F(CompareCaseInsensitiveTest, MatchesDifferentCase) {
    EXPECT_TRUE(StringUtils::compareCaseInsensitive("HELLO", "hello"));
    EXPECT_TRUE(StringUtils::compareCaseInsensitive("HeLLo", "hEllO"));
}

TEST_F(CompareCaseInsensitiveTest, ReturnsFalseForDifferentStrings) {
    EXPECT_FALSE(StringUtils::compareCaseInsensitive("hello", "world"));
}

TEST_F(CompareCaseInsensitiveTest, HandlesEmptyStrings) {
    EXPECT_TRUE(StringUtils::compareCaseInsensitive("", ""));
}

TEST_F(CompareCaseInsensitiveTest, EmptyVsNonEmpty) {
    EXPECT_FALSE(StringUtils::compareCaseInsensitive("", "a"));
    EXPECT_FALSE(StringUtils::compareCaseInsensitive("a", ""));
}

// Tests for getStringWithinChar()
class GetStringWithinCharTest : public ::testing::Test {};

TEST_F(GetStringWithinCharTest, ExtractsStringBetweenDelimiters) {
    EXPECT_EQ(StringUtils::getStringWithinChar("/path/to/file/", '/'), "path/to/file");
}

TEST_F(GetStringWithinCharTest, ExtractsQuotedString) {
    EXPECT_EQ(StringUtils::getStringWithinChar("\"hello world\"", '"'), "hello world");
}

TEST_F(GetStringWithinCharTest, ReturnsEmptyForSingleDelimiter) {
    EXPECT_EQ(StringUtils::getStringWithinChar("/single", '/'), "");
}

TEST_F(GetStringWithinCharTest, ReturnsEmptyForNoDelimiter) {
    EXPECT_EQ(StringUtils::getStringWithinChar("noslash", '/'), "");
}

TEST_F(GetStringWithinCharTest, ReturnsEmptyForAdjacentDelimiters) {
    EXPECT_EQ(StringUtils::getStringWithinChar("//", '/'), "");
}

TEST_F(GetStringWithinCharTest, HandlesEmptyString) {
    EXPECT_EQ(StringUtils::getStringWithinChar("", '/'), "");
}

// Tests for removeCharsFromString()
class RemoveCharsFromStringTest : public ::testing::Test {};

TEST_F(RemoveCharsFromStringTest, RemovesSingleChar) {
    std::string s = "hello";
    StringUtils::removeCharsFromString(s, "l");
    EXPECT_EQ(s, "heo");
}

TEST_F(RemoveCharsFromStringTest, RemovesMultipleChars) {
    std::string s = "hello, world!";
    StringUtils::removeCharsFromString(s, " ,!");
    EXPECT_EQ(s, "helloworld");
}

TEST_F(RemoveCharsFromStringTest, HandlesNoMatch) {
    std::string s = "hello";
    StringUtils::removeCharsFromString(s, "xyz");
    EXPECT_EQ(s, "hello");
}

TEST_F(RemoveCharsFromStringTest, HandlesEmptyCharsToRemove) {
    std::string s = "hello";
    StringUtils::removeCharsFromString(s, "");
    EXPECT_EQ(s, "hello");
}

TEST_F(RemoveCharsFromStringTest, HandlesEmptyString) {
    std::string s = "";
    StringUtils::removeCharsFromString(s, "abc");
    EXPECT_EQ(s, "");
}

// Tests for removeComment()
class RemoveCommentTest : public ::testing::Test {};

TEST_F(RemoveCommentTest, RemovesTrailingComment) {
    std::string s = "value # this is a comment";
    StringUtils::removeComment(s);
    EXPECT_EQ(s, "value ");
}

TEST_F(RemoveCommentTest, RemovesCommentAtStart) {
    std::string s = "# full line comment";
    StringUtils::removeComment(s);
    EXPECT_EQ(s, "");
}

TEST_F(RemoveCommentTest, PreservesStringWithoutComment) {
    std::string s = "no comment here";
    StringUtils::removeComment(s);
    EXPECT_EQ(s, "no comment here");
}

TEST_F(RemoveCommentTest, HandlesEmptyString) {
    std::string s = "";
    StringUtils::removeComment(s);
    EXPECT_EQ(s, "");
}

TEST_F(RemoveCommentTest, HandlesMultipleHashes) {
    std::string s = "value # comment # more";
    StringUtils::removeComment(s);
    EXPECT_EQ(s, "value ");
}

// Tests for cleanPublisherString()
class CleanPublisherStringTest : public ::testing::Test {};

TEST_F(CleanPublisherStringTest, RemovesTrailingPeriod) {
    std::string s = "Sony.";
    StringUtils::cleanPublisherString(s);
    EXPECT_EQ(s, "Sony");
}

TEST_F(CleanPublisherStringTest, RemovesTrailingSpace) {
    std::string s = "Konami ";
    StringUtils::cleanPublisherString(s);
    EXPECT_EQ(s, "Konami");
}

TEST_F(CleanPublisherStringTest, RemovesMultipleTrailingChars) {
    std::string s = "Square Enix. . ";
    StringUtils::cleanPublisherString(s);
    EXPECT_EQ(s, "Square Enix");
}

TEST_F(CleanPublisherStringTest, PreservesCleanString) {
    std::string s = "Capcom";
    StringUtils::cleanPublisherString(s);
    EXPECT_EQ(s, "Capcom");
}

TEST_F(CleanPublisherStringTest, HandlesEmptyString) {
    std::string s = "";
    StringUtils::cleanPublisherString(s);
    EXPECT_EQ(s, "");
}

TEST_F(CleanPublisherStringTest, HandlesAllPeriodsAndSpaces) {
    std::string s = ".. . ";
    StringUtils::cleanPublisherString(s);
    EXPECT_EQ(s, "");
}

// Tests for getTokens()
class GetTokensTest : public ::testing::Test {};

TEST_F(GetTokensTest, SplitsByDelimiter) {
    auto tokens = StringUtils::getTokens("a:b:c", ':');
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], "b");
    EXPECT_EQ(tokens[2], "c");
}

TEST_F(GetTokensTest, SkipsEmptyTokens) {
    auto tokens = StringUtils::getTokens("a::b", ':');
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], "b");
}

TEST_F(GetTokensTest, HandlesNoDelimiter) {
    auto tokens = StringUtils::getTokens("hello", ':');
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "hello");
}

TEST_F(GetTokensTest, HandlesEmptyString) {
    auto tokens = StringUtils::getTokens("", ':');
    EXPECT_TRUE(tokens.empty());
}

TEST_F(GetTokensTest, HandlesOnlyDelimiters) {
    auto tokens = StringUtils::getTokens(":::", ':');
    EXPECT_TRUE(tokens.empty());
}

TEST_F(GetTokensTest, SplitsPathComponents) {
    auto tokens = StringUtils::getTokens("/usr/local/bin", '/');
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "usr");
    EXPECT_EQ(tokens[1], "local");
    EXPECT_EQ(tokens[2], "bin");
}

// Tests for getToken()
class GetTokenTest : public ::testing::Test {};

TEST_F(GetTokenTest, ReturnsFirstToken) {
    EXPECT_EQ(StringUtils::getToken("a,b,c", ',', 0), "a");
}

TEST_F(GetTokenTest, ReturnsMiddleToken) {
    EXPECT_EQ(StringUtils::getToken("a,b,c", ',', 1), "b");
}

TEST_F(GetTokenTest, ReturnsLastToken) {
    EXPECT_EQ(StringUtils::getToken("a,b,c", ',', 2), "c");
}

TEST_F(GetTokenTest, ReturnsEmptyForOutOfRange) {
    EXPECT_EQ(StringUtils::getToken("a,b,c", ',', 5), "");
}

TEST_F(GetTokenTest, ReturnsEmptyForNegativePosition) {
    EXPECT_EQ(StringUtils::getToken("a,b,c", ',', -1), "");
}

TEST_F(GetTokenTest, PreservesEmptyTokens) {
    EXPECT_EQ(StringUtils::getToken("a,,c", ',', 1), "");
}

TEST_F(GetTokenTest, HandlesNoDelimiter) {
    EXPECT_EQ(StringUtils::getToken("hello", ',', 0), "hello");
    EXPECT_EQ(StringUtils::getToken("hello", ',', 1), "");
}

TEST_F(GetTokenTest, HandlesEmptyString) {
    EXPECT_EQ(StringUtils::getToken("", ',', 0), "");
}

// Tests for formatFloat()
class FormatFloatTest : public ::testing::Test {};

TEST_F(FormatFloatTest, FormatsWithZeroPrecision) {
    EXPECT_EQ(StringUtils::formatFloat(3.14159f, 0), "3");
}

TEST_F(FormatFloatTest, FormatsWithTwoPrecision) {
    EXPECT_EQ(StringUtils::formatFloat(3.14159f, 2), "3.14");
}

TEST_F(FormatFloatTest, FormatsWithHighPrecision) {
    std::string result = StringUtils::formatFloat(1.5f, 5);
    EXPECT_EQ(result, "1.50000");
}

TEST_F(FormatFloatTest, FormatsNegativeNumber) {
    EXPECT_EQ(StringUtils::formatFloat(-2.5f, 1), "-2.5");
}

TEST_F(FormatFloatTest, FormatsZero) {
    EXPECT_EQ(StringUtils::formatFloat(0.0f, 2), "0.00");
}

TEST_F(FormatFloatTest, FormatsLargeNumber) {
    EXPECT_EQ(StringUtils::formatFloat(1234567.89f, 1), "1234567.9");
}

// =============================================================================
// Tests for inline functions (defined in string_utils.h)
// =============================================================================

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
