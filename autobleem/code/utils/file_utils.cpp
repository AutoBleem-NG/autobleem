#include "file_utils.h"
#include "string_utils.h"
#include "path_utils.h"
#include "../environment.h"
#include "../log.h"

#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

// 512KB buffer balances I/O efficiency vs memory usage on PSC's limited RAM
#define FILE_BUFFER_SIZE 524288

const Sep sep;

string operator+(const string &leftside, Sep s) {
    string ret = leftside;
    ret += s;
    return ret;
}

void operator+=(string &leftside, Sep) {
    if (leftside.size() > 0) {
        char lastChar = leftside.back();
        if (lastChar != PathUtils::separator)
            leftside += PathUtils::separator;
    }
}

namespace FileUtils {

void DirEntry::print() const { PLOG_DEBUG << (isDir ? "Dir: " : "File: ") << name << endl; }

bool DirEntry::sortByName(const DirEntry &i, const DirEntry &j) { return SortByCaseInsensitive(i.name, j.name); }

vector<string> readTextFile(const string &filePath, bool removeCRLF) {
    ifstream file;
    string line;
    vector<string> contents;

    file.open(filePath);
    if (!file.good()) {
        PLOG_ERROR << "Error opening file: " << filePath;
        return contents;
    }

    while (getline(file, line)) {
        if (removeCRLF) {
            size_t crPos = line.find('\r');
            size_t lfPos = line.find('\n');
            if (crPos != string::npos || lfPos != string::npos) {
                size_t endPos = min(crPos, lfPos);
                if (endPos != string::npos) {
                    line = line.substr(0, endPos);
                }
            }
        }
        contents.emplace_back(line);
    }
    file.close();

    return contents;
}

bool writeTextFile(const string &filePath, const vector<string> &lines, bool appendLineEnding) {
    ofstream os(filePath, ios_base::trunc);
    if (!os.is_open()) {
        return false;
    }

    for (const string &s : lines) {
        if (appendLineEnding) {
            os << s << endl;
        } else {
            os << s;
        }
    }
    os << flush;
    return true;
}

istream &getlineRemoveCR(istream &is, string &str) {
    getline(is, str);
    if (!str.empty() && str.back() == '\r') {
        str.pop_back();
    }
    return is;
}

bool exists(const string &_name) {
    auto name = fixPath(_name);
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

bool isDirectory(const string &path) {
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        return false;
    }
    return S_ISDIR(path_stat.st_mode);
}

bool copy(const string &source, const string &dest) {
    ifstream infile(source, ios::binary);
    if (!infile.good()) {
        return false;
    }

    ofstream outfile(dest, ios::binary);
    if (!outfile.good()) {
        return false;
    }

    // Use vector for automatic memory management
    vector<char> buffer(FILE_BUFFER_SIZE);

    while (true) {
        streamsize bytesRead = infile.readsome(buffer.data(), FILE_BUFFER_SIZE);
        if (bytesRead == 0)
            break;
        outfile.write(buffer.data(), bytesRead);
    }
    outfile.flush();

    return true;
}

bool copyFile(const string &pathFrom, const string &pathTo) { return copy(pathFrom, pathTo); }

bool removeFile(const string &path) { return remove(path.c_str()) == 0; }

bool renameFile(const string &pathFrom, const string &pathTo) { return rename(pathFrom.c_str(), pathTo.c_str()) == 0; }

bool createDir(const string &_name) {
    auto name = fixPath(_name);
    const int dir_err = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return (-1 != dir_err);
}

bool createDirRecursive(const string &path) {
    string fixedPath = fixPath(path);
    if (fixedPath.empty()) {
        return false;
    }

    // Check if already exists
    if (exists(fixedPath) && isDirectory(fixedPath)) {
        return true;
    }

    // Build path incrementally and create each level
    string current;
    size_t pos = 0;

    // Handle absolute paths
    if (fixedPath[0] == '/') {
        current = "/";
        pos = 1;
    }

    while (pos < fixedPath.length()) {
        size_t nextSlash = fixedPath.find('/', pos);
        if (nextSlash == string::npos) {
            nextSlash = fixedPath.length();
        }

        current += fixedPath.substr(pos, nextSlash - pos);

        if (!current.empty() && !exists(current)) {
            if (!createDir(current)) {
                return false;
            }
        }

        if (nextSlash < fixedPath.length()) {
            current += "/";
        }
        pos = nextSlash + 1;
    }

    return true;
}

bool ensureParentDirExists(const string &filePath) {
    string dirPath = getDirNameFromPath(filePath);
    if (dirPath.empty() || dirPath == ".") {
        return true; // Current directory, nothing to create
    }
    return createDirRecursive(dirPath);
}

int rmDir(string path) {
    path = fixPath(path);
    DIR *d = opendir(path.c_str());
    int r = -1;

    if (d == nullptr) {
        return r;
    }

    r = 0;
    struct dirent *p;

    while (!r && (p = readdir(d))) {
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) {
            continue;
        }

        string fullPath = path + "/" + p->d_name;
        struct stat statbuf;

        if (stat(fullPath.c_str(), &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                r = rmDir(fullPath);
            } else {
                r = unlink(fullPath.c_str());
            }
        }
    }
    closedir(d);

    if (r == 0) {
        r = rmdir(path.c_str());
    }
    return r;
}

bool removeDirAndContents(const string &path) {
    auto files = diru_FilesOnly(path);
    for (auto &file : files) {
        removeFile(path + sep + file.name);
    }

    auto dirs = diru_DirsOnly(path);
    for (auto &dir : dirs) {
        removeDirAndContents(path + sep + dir.name);
    }

    return (rmdir(path.c_str()) == 0);
}

DirEntries dir(string path) {
    path = fixPath(removeSeparatorFromEndOfPath(path));
    DirEntries result;
    DIR *dirPtr = opendir(path.c_str());
    if (dirPtr != nullptr) {
        struct dirent *entry = readdir(dirPtr);
        while (entry != nullptr) {
            DirEntry obj(entry->d_name, entry->d_type == DT_DIR);
            result.push_back(obj);
            entry = readdir(dirPtr);
        }
        closedir(dirPtr);
    }
    sort(result.begin(), result.end(), DirEntry::sortByName);
    return result;
}

DirEntries diru(string path) {
    path = fixPath(removeSeparatorFromEndOfPath(path));
    DirEntries result;
    DIR *dirPtr = opendir(path.c_str());
    if (dirPtr != nullptr) {
        struct dirent *entry = readdir(dirPtr);
        while (entry != nullptr) {
            DirEntry obj(entry->d_name, isDirectory(path + sep + entry->d_name));
            if (entry->d_name[0] != '.') {
                result.push_back(obj);
            }
            entry = readdir(dirPtr);
        }
        closedir(dirPtr);
    }
    sort(result.begin(), result.end(), DirEntry::sortByName);
    return result;
}

DirEntries diru_DirsOnly(string path) {
    auto temp = diru(path);
    DirEntries ret;
    copy_if(begin(temp), end(temp), back_inserter(ret), [](const DirEntry &dir) { return dir.isDir; });
    return ret;
}

DirEntries diru_FilesOnly(string path) {
    auto temp = diru(path);
    DirEntries ret;
    copy_if(begin(temp), end(temp), back_inserter(ret), [](const DirEntry &dir) { return !dir.isDir; });
    return ret;
}

string fixPath(string path) { return PathUtils::fixPath(path); }

string removeSeparatorFromEndOfPath(const string &path) { return PathUtils::removeSeparatorFromEndOfPath(path); }

string getFileNameFromPath(const string &path) { return PathUtils::getFileNameFromPath(path); }

string getDirNameFromPath(const string &path) { return PathUtils::getDirNameFromPath(path); }

string getFileExtension(const string &fileName) { return PathUtils::getFileExtension(fileName); }

string getFileNameWithoutExtension(const string &filename) { return PathUtils::getFileNameWithoutExtension(filename); }

string removeGamesPathFromFrontOfPath(const string &path) {
    string gamesDir = Env::getPathToGamesDir() + sep;
    size_t len = gamesDir.size();
    if (path.compare(0, len, gamesDir) == 0)
        return string(path).erase(0, len);
    return path;
}

string removeDotFromExtension(const string &ext) {
    if (!ext.empty() && ext[0] == '.')
        return ext.substr(1);
    return ext;
}

string addDotToExtension(const string &ext) {
    if (!ext.empty() && ext[0] != '.')
        return string(".") + ext;
    return ext;
}

bool matchExtension(string path, string ext) {
    path = fixPath(path);
    string fileExt = getFileExtension(path);
    if (fileExt.empty()) {
        return false;
    }
    // getFileExtension returns without dot, so ensure both are compared without dots
    string targetExt = removeDotFromExtension(ext);
    return ReturnLowerCase(fileExt) == ReturnLowerCase(targetExt);
}

DirEntries getFilesWithExtension(const string &path, const DirEntries &entries, const vector<string> &extensions) {
    DirEntries fileList;
    string fileExt;
    for (const auto &entry : entries) {
        if (isDirectory(path + sep + entry.name))
            continue;
        fileExt = ReturnLowerCase(getFileExtension(entry.name));
        if (find(extensions.begin(), extensions.end(), fileExt) != extensions.end()) {
            fileList.push_back(entry);
        }
    }
    return fileList;
}

string findFirstFile(string ext, string path) {
    path = fixPath(path);
    DirEntries entries = diru(path);
    for (const DirEntry &entry : entries) {
        if (matchExtension(entry.name, ext)) {
            return entry.name;
        }
    }
    return "";
}

vector<string> cueToBinList(string cueFile) {
    vector<string> binList;
    FILE *fp;
    char *cline = nullptr;
    string line;
    size_t length = 0;
    ssize_t read;

    fp = fopen(cueFile.c_str(), "r");
    if (fp == nullptr) {
        PLOG_ERROR << "Failed to open cue file: " << cueFile;
        return binList;
    }

    while ((read = getline(&cline, &length, fp)) != -1) {
        line = cline;
        line = trim(line);
        if (line.substr(0, 4) == "FILE") {
            binList.push_back(StringUtils::getStringWithinChar(line, '"').c_str());
        }
    }

    fclose(fp);

    if (cline) {
        free(cline);
    }

    return binList;
}

string replaceTheseCharsWithThisChar(string str, const string &charsToReplace, char replacementChar) {
    auto isBadChar = [&](char c) { return charsToReplace.find(c) != string::npos; };
    replace_if(begin(str), end(str), isBadChar, replacementChar);
    return str;
}

bool fixCommaInDirOrFileName(const string &path, DirEntry *entry) {
    if (entry->name.find(",") != string::npos) {
        string newName = entry->name;
        StringUtils::replaceAll(newName, ",", "-");
        renameFile(path + sep + entry->name, path + sep + newName);
        entry->name = newName;
        return true;
    }
    return false;
}

bool isPBPFile(const string &path) { return matchExtension(path, "pbp"); }

void generateM3UForDirectory(const string &path, string basename) {
    // Strip image extensions so we don't end up with "<name>.chd.m3u".
    if (isPBPFile(basename) || StringUtils::compareCaseInsensitive(getFileExtension(basename), "chd") ||
        StringUtils::compareCaseInsensitive(getFileExtension(basename), "cue") ||
        StringUtils::compareCaseInsensitive(getFileExtension(basename), "bin") ||
        StringUtils::compareCaseInsensitive(getFileExtension(basename), "img")) {
        basename = getFileNameWithoutExtension(basename);
    }
    vector<string> files;
    DirEntries filesInPath = diru_FilesOnly(path);
    for (const DirEntry &entry : filesInPath) {
        string ext = getFileExtension(entry.name);
        if (StringUtils::compareCaseInsensitive(ext, "pbp") || StringUtils::compareCaseInsensitive(ext, "cue") ||
            StringUtils::compareCaseInsensitive(ext, "chd"))
            files.push_back(entry.name);
    }
    std::sort(files.begin(), files.end());
    if (files.size() > 1) {
        // Remove any stale .m3u files first so re-scans don't accumulate
        // playlists with different basenames pointing at the same discs.
        for (const DirEntry &entry : filesInPath) {
            if (StringUtils::compareCaseInsensitive(getFileExtension(entry.name), "m3u"))
                removeFile(fixPath(path) + sep + entry.name);
        }
        string m3uName = fixPath(path) + sep + basename + ".m3u";
        ofstream os(m3uName);
        for (const string &file : files) {
            os << file << endl;
        }
    }
}

bool isAGameFile(const string &filename) {
    if (matchExtension(filename, EXT_BIN))
        return true;
    if (matchExtension(filename, EXT_PBP))
        return true;
    if (matchExtension(filename, EXT_IMG))
        return true;
    if (matchExtension(filename, EXT_CHD))
        return true;
    return false;
}

ImageType getGameFileImageType(const string &filename) {
    if (matchExtension(filename, EXT_BIN))
        return IMAGE_BIN;
    if (matchExtension(filename, EXT_PBP))
        return IMAGE_PBP;
    if (matchExtension(filename, EXT_IMG))
        return IMAGE_IMG;
    if (matchExtension(filename, EXT_CHD))
        return IMAGE_CHD;
    return IMAGE_NO_GAME_FOUND;
}

bool imageTypeUsesACueFile(ImageType imageType) { return (imageType == IMAGE_BIN || imageType == IMAGE_IMG); }

bool thereIsAGameFile(const DirEntries &entries) {
    return any_of(begin(entries), end(entries),
                  [](const DirEntry &entry) { return !entry.isDir && isAGameFile(entry.name); });
}

bool thereIsAGameFile(const string &dirpath) { return thereIsAGameFile(diru(dirpath)); }

bool thereIsASubDir(const DirEntries &entries) {
    return any_of(begin(entries), end(entries), [](const DirEntry &entry) { return entry.isDir; });
}

bool thereIsASubDir(const string &dirpath) { return thereIsASubDir(diru(dirpath)); }

tuple<ImageType, string> getGameFile(const DirEntries &entries) {
    auto iter = find_if(begin(entries), end(entries), [](const DirEntry &entry) { return isAGameFile(entry.name); });
    if (iter != end(entries))
        return make_tuple(getGameFileImageType(iter->name), iter->name);
    return make_tuple(IMAGE_NO_GAME_FOUND, "");
}

tuple<ImageType, string> getGameFile(const string &dirpath) { return getGameFile(diru(dirpath)); }

} // namespace FileUtils
