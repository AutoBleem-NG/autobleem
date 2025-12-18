#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// String utility functions for trimming, case conversion, tokenization, and comparison.
// Used throughout AutoBleem for parsing game metadata files, configuration values,
// and normalizing user-visible strings.

// Removes leading whitespace (spaces, tabs, newlines) from a string in place.
// Returns a reference to the modified string to allow chaining: trim(ltrim(s)).
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    return s;
}

// Removes trailing whitespace (spaces, tabs, newlines) from a string in place.
// Returns a reference to the modified string to allow chaining.
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

// Removes whitespace from both ends of a string in place.
// Combines ltrim and rtrim for convenience.
static inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

// Converts all or part of a string to lowercase in place.
// If nchars is 0 (default), converts the entire string.
// If nchars > 0, converts only the first nchars characters.
// Used for case-insensitive comparisons and normalizing game titles.
static inline std::string &lcase(std::string &s, std::size_t nchars = 0) {
    if (nchars == 0) {
        nchars = s.length();
    }
    for (std::size_t i = 0; i < nchars; i++) {
        s[i] = tolower(static_cast<unsigned char>(s[i]));
    }
    return s;
}

// Converts all or part of a string to uppercase in place.
// If nchars is 0 (default), converts the entire string.
// If nchars > 0, converts only the first nchars characters.
static inline std::string &ucase(std::string &s, std::size_t nchars = 0) {
    if (nchars == 0) {
        nchars = s.length();
    }
    for (std::size_t i = 0; i < nchars; i++) {
        s[i] = toupper(static_cast<unsigned char>(s[i]));
    }
    return s;
}

// Returns a lowercase copy of the input string without modifying the original.
// Useful when the original string must remain unchanged (e.g., display vs. comparison).
[[nodiscard]] static inline std::string ReturnLowerCase(const std::string &s) {
    std::string temp = s;
    for (auto &c : temp) {
        c = tolower(static_cast<unsigned char>(c));
    }
    return temp;
}

// Returns an uppercase copy of the input string without modifying the original.
[[nodiscard]] static inline std::string ReturnUpperCase(const std::string &s) {
    std::string temp = s;
    for (auto &c : temp) {
        c = toupper(static_cast<unsigned char>(c));
    }
    return temp;
}

// Comparison function for case-insensitive sorting with std::sort.
// Returns true if 'left' should come before 'right' alphabetically (ignoring case).
// Example usage: std::sort(games.begin(), games.end(), SortByCaseInsensitive);
[[nodiscard]] static inline bool SortByCaseInsensitive(const std::string &left, const std::string &right) {
    return ReturnLowerCase(left) < ReturnLowerCase(right);
}

// Non-inline string utilities implemented in string_utils.cpp.
// These functions are more complex or used less frequently than the inline versions above.
namespace StringUtils {

// Escapes special characters in a string for safe storage in delimited formats.
// Converts '|' to '||' and ',' to '|@' to avoid conflicts with CSV-style parsing.
// Used when saving game metadata that may contain these characters (e.g., game titles).
[[nodiscard]] std::string escape(const std::string &input);

// Decodes a string that was previously escaped with escape().
// Reverses the escaping: '|@' becomes ',' and '||' becomes '|'.
// Order matters: decode |@ first, then ||, to avoid incorrectly expanding |||@ to ||,.
[[nodiscard]] std::string decode(const std::string &input);

// Replaces all occurrences of 'from' with 'to' in the given string.
// Handles the case where 'to' contains 'from' by advancing past each replacement.
// Does nothing if 'from' is empty (prevents infinite loop).
void replaceAll(std::string &str, const std::string &from, const std::string &to);

// Returns true if the input string contains only ASCII digits (0-9).
// Returns false for null input, empty string, or strings containing any non-digit.
// Used to validate numeric IDs and indices parsed from configuration files.
[[nodiscard]] bool isInteger(const char *input);

// Compares two strings for equality, ignoring case differences.
// Returns true if the strings are equal when both are converted to lowercase.
[[nodiscard]] bool compareCaseInsensitive(const std::string &first, const std::string &second);

// Extracts the substring between the first and last occurrence of a delimiter character.
// For example, getStringWithinChar("path/to/file.txt", '/') returns "to/file.txt".
// Returns empty string if delimiter not found or only found once.
[[nodiscard]] std::string getStringWithinChar(const std::string &s, char del);

// Removes all occurrences of any character in 'charsToRemove' from the string.
// For example, removeCharsFromString(str, ".,!") removes all periods, commas, and exclamation marks.
void removeCharsFromString(std::string &str, const std::string &charsToRemove);

// Removes shell-style comments from a string (everything from '#' to end of line).
// Used when parsing configuration files that support comment syntax.
void removeComment(std::string &str);

// Removes trailing periods and spaces from a publisher string.
// Some game metadata sources include trailing punctuation that looks messy in the UI.
void cleanPublisherString(std::string &pub);

// Splits a string by a delimiter and returns non-empty tokens.
// For example, getTokens("a:b::c", ':') returns {"a", "b", "c"} (empty tokens skipped).
// Used for parsing path components and colon-separated configuration values.
[[nodiscard]] std::vector<std::string> getTokens(const std::string &str, char delim);

// Returns the token at a specific position when splitting by delimiter.
// Position is 0-indexed. Returns empty string if position is out of range or negative.
// Unlike getTokens, this preserves empty tokens for accurate positional access.
[[nodiscard]] std::string getToken(const std::string &input, char delimiter, int position);

// Formats a floating-point value as a string with the specified precision.
// Used for displaying storage space, percentages, and other numeric values in the UI.
[[nodiscard]] std::string formatFloat(float value, int precision);

} // namespace StringUtils
