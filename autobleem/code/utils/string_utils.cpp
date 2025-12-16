#include "string_utils.h"
#include <cctype>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace StringUtils {

std::string escape(std::string input) {
    replaceAll(input, "|", "||");
    replaceAll(input, ",", "|@");
    return input;
}

std::string decode(std::string input) {
    replaceAll(input, "|@", ",");
    replaceAll(input, "||", "|");
    return input;
}

void replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) {
        return;
    }
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

bool isInteger(const char *input) {
    if (!input) {
        return false;
    }
    size_t len = std::strlen(input);
    for (size_t i = 0; i < len; i++) {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) {
            return false;
        }
    }
    return len > 0;
}

bool compareCaseInsensitive(std::string first, std::string second) {
    return ReturnLowerCase(first) == ReturnLowerCase(second);
}

std::string getStringWithinChar(std::string s, char del) {
    size_t first = s.find(del);
    size_t last = s.find_last_of(del);
    if (first == std::string::npos || last == std::string::npos || first >= last) {
        return "";
    }
    return s.substr(first + 1, last - first - 1);
}

void removeCharsFromString(std::string &str, std::string charsToRemove) {
    for (char ch : charsToRemove) {
        str.erase(std::remove(str.begin(), str.end(), ch), str.end());
    }
}

void removeComment(std::string &str) {
    size_t commentPos = str.find("#");
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
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find(delimiter, start);
    }
    tokens.push_back(input.substr(start));

    return (position < static_cast<int>(tokens.size())) ? tokens[position] : "";
}

std::string formatFloat(float value, int precision) {
    std::ostringstream stringStream;
    stringStream << std::fixed << std::setprecision(precision) << value;
    return stringStream.str();
}

} // namespace StringUtils
