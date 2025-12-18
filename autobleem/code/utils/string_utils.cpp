#include "string_utils.h"
#include <cctype>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace StringUtils {

std::string escape(const std::string &input) {
    std::string result = input;
    // Order matters: escape the escape character '|' first, then the delimiter ','
    replaceAll(result, "|", "||");
    replaceAll(result, ",", "|@");
    return result;
}

std::string decode(const std::string &input) {
    std::string result = input;
    // Order matters: decode the escaped delimiter first, then the escape character.
    // If we decoded || first, the sequence "|@" might incorrectly become part of "||@".
    replaceAll(result, "|@", ",");
    replaceAll(result, "||", "|");
    return result;
}

void replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) {
        return;
    }
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        // Advance past the replacement to handle cases where 'to' contains 'from'
        pos += to.length();
    }
}

bool isInteger(const char *input) {
    if (input == nullptr) {
        return false;
    }
    size_t len = std::strlen(input);
    if (len == 0) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) {
            return false;
        }
    }
    return true;
}

bool compareCaseInsensitive(const std::string &first, const std::string &second) {
    return ReturnLowerCase(first) == ReturnLowerCase(second);
}

std::string getStringWithinChar(const std::string &s, char del) {
    size_t first = s.find(del);
    size_t last = s.find_last_of(del);
    if (first == std::string::npos || last == std::string::npos || first >= last) {
        return "";
    }
    return s.substr(first + 1, last - first - 1);
}

void removeCharsFromString(std::string &str, const std::string &charsToRemove) {
    for (char ch : charsToRemove) {
        str.erase(std::remove(str.begin(), str.end(), ch), str.end());
    }
}

void removeComment(std::string &str) {
    size_t commentPos = str.find('#');
    if (commentPos != std::string::npos) {
        str.erase(commentPos);
    }
}

void cleanPublisherString(std::string &pub) {
    while (!pub.empty() && (pub.back() == '.' || pub.back() == ' ')) {
        pub.pop_back();
    }
}

std::vector<std::string> getTokens(const std::string &str, char delim) {
    std::istringstream ss(str);
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(ss, token, delim)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string getToken(const std::string &input, char delimiter, int position) {
    if (position < 0) {
        return "";
    }

    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find(delimiter, start);
    }
    tokens.push_back(input.substr(start));

    size_t pos = static_cast<size_t>(position);
    return (pos < tokens.size()) ? tokens[pos] : "";
}

std::string formatFloat(float value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

} // namespace StringUtils
