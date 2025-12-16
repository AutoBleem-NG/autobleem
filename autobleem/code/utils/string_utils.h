#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// String utility functions for trimming, case conversion, and comparison

// trim from start (modifies string in place)
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    return s;
}

// trim from end (modifies string in place)
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

// trim from both ends (modifies string in place)
static inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

// converts all, or part, of the passed string to lower case (modifies string in place)
static inline std::string &lcase(std::string &s, std::size_t nchars = 0) {
    if (nchars == 0)
        nchars = s.length();
    for (std::size_t i = 0; i < nchars; i++) {
        s[i] = tolower(static_cast<unsigned char>(s[i]));
    }
    return s;
}

// converts all, or part, of the passed string to upper case (modifies string in place)
static inline std::string &ucase(std::string &s, std::size_t nchars = 0) {
    if (nchars == 0)
        nchars = s.length();
    for (std::size_t i = 0; i < nchars; i++) {
        s[i] = toupper(static_cast<unsigned char>(s[i]));
    }
    return s;
}

// returns a lower case copy of the string (original not modified)
static inline std::string ReturnLowerCase(const std::string &s) {
    std::string temp = s;
    for (auto &c : temp) {
        c = tolower(static_cast<unsigned char>(c));
    }
    return temp;
}

// returns an upper case copy of the string (original not modified)
static inline std::string ReturnUpperCase(const std::string &s) {
    std::string temp = s;
    for (auto &c : temp) {
        c = toupper(static_cast<unsigned char>(c));
    }
    return temp;
}

// comparison function for case-insensitive sorting
static inline bool SortByCaseInsensitive(const std::string &left, const std::string &right) {
    return ReturnLowerCase(left) < ReturnLowerCase(right);
}

// Non-inline string utilities (implemented in string_utils.cpp)
namespace StringUtils {
// String encoding/decoding
std::string escape(std::string input);
std::string decode(std::string input);

// String replacement
void replaceAll(std::string &str, const std::string &from, const std::string &to);

// String validation
bool isInteger(const char *input);
bool compareCaseInsensitive(std::string first, std::string second);

// String extraction and manipulation
std::string getStringWithinChar(std::string s, char del);
void removeCharsFromString(std::string &str, std::string charsToRemove);
void removeComment(std::string &str);
void cleanPublisherString(std::string &pub);

// String tokenization
std::vector<std::string> getTokens(const std::string &str, char delim);

// String parsing and formatting
std::string getToken(const std::string &input, char delimiter, int position);
std::string formatFloat(float value, int precision);
} // namespace StringUtils
