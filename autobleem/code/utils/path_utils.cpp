#include "path_utils.h"
#include <libgen.h>
#include <cstring>

using namespace std;

// Platform-specific separator
#ifdef _WIN32
const char PathUtils::separator = '\\';
#else
const char PathUtils::separator = '/';
#endif

namespace PathUtils {

string getFileExtension(const string &path) {
    size_t i = path.rfind('.', path.length());
    if (i != string::npos) {
        return path.substr(i + 1, path.length() - i);
    }
    return "";
}

string getFileNameWithoutExtension(const string &filename) {
    size_t indexBeforeDot = filename.find_last_of(".");
    if (indexBeforeDot != string::npos) {
        return filename.substr(0, indexBeforeDot);
    }
    return filename;
}

string getFileNameFromPath(const string &path) {
    string result = "";
    char *cstr = new char[path.length() + 1];
    strcpy(cstr, path.c_str());
    char *base = basename(cstr);
    result += base;
    delete[] cstr;
    return result;
}

string getDirNameFromPath(const string &path) {
    string result = "";
    char *cstr = new char[path.length() + 1];
    strcpy(cstr, path.c_str());
    char *base = dirname(cstr);
    result += base;
    delete[] cstr;
    return result;
}

string joinPath(const string &dir, const string &file) {
    if (dir.empty()) return file;
    if (file.empty()) return dir;

    string result = dir;
    if (result.back() != separator) {
        result += separator;
    }
    result += file;
    return result;
}

string removeSeparatorFromEndOfPath(const string &path) {
    string ret = path;
    if (ret.length() > 0 && ret.back() == separator) {
        ret.pop_back();
    }
    return ret;
}

string fixPath(const string &path) {
    string result = path;
    // Remove leading/trailing spaces
    size_t start = result.find_first_not_of(" \n\r\t\f\v");
    if (start == string::npos) {
        return "";
    }
    size_t end = result.find_last_not_of(" \n\r\t\f\v");
    result = result.substr(start, end - start + 1);

    // Remove trailing separator
    if (result.size() > 0 && result.back() == separator) {
        result.pop_back();
    }

    return result;
}

} // namespace PathUtils
