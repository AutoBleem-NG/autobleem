#include "path_utils.h"

#include <libgen.h>
#include <vector>

namespace PathUtils {

#ifdef _WIN32
const char separator = '\\';
#else
const char separator = '/';
#endif

std::string getFileExtension(const std::string &path) {
    size_t i = path.rfind('.', path.length());
    if (i != std::string::npos) {
        return path.substr(i + 1, path.length() - i);
    }
    return "";
}

std::string getFileNameWithoutExtension(const std::string &filename) {
    size_t indexBeforeDot = filename.find_last_of(".");
    if (indexBeforeDot != std::string::npos) {
        return filename.substr(0, indexBeforeDot);
    }
    return filename;
}

std::string getFileNameFromPath(const std::string &path) {
    std::vector<char> buffer(path.begin(), path.end());
    buffer.push_back('\0');
    return std::string(basename(buffer.data()));
}

std::string getDirNameFromPath(const std::string &path) {
    std::vector<char> buffer(path.begin(), path.end());
    buffer.push_back('\0');
    return std::string(dirname(buffer.data()));
}

std::string joinPath(const std::string &dir, const std::string &file) {
    if (dir.empty()) {
        return file;
    }
    if (file.empty()) {
        return dir;
    }

    std::string result = dir;
    if (result.back() != separator) {
        result += separator;
    }
    result += file;
    return result;
}

std::string removeSeparatorFromEndOfPath(const std::string &path) {
    std::string ret = path;
    if (!ret.empty() && ret.back() == separator) {
        ret.pop_back();
    }
    return ret;
}

std::string fixPath(const std::string &path) {
    std::string result = path;

    size_t start = result.find_first_not_of(" \n\r\t\f\v");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = result.find_last_not_of(" \n\r\t\f\v");
    result = result.substr(start, end - start + 1);

    if (!result.empty() && result.back() == separator) {
        result.pop_back();
    }

    return result;
}

} // namespace PathUtils
