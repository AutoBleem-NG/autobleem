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

// 0.5MB buffer for file copy operations
#define FILE_BUFFER_SIZE 524288

// ========================================
// Separator Helper Implementation
// ========================================

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

// ========================================
// Directory Entry Methods
// ========================================

void DirEntry::print() const { PLOG_DEBUG << (isDir ? "Dir: " : "File: ") << name << endl; }

bool DirEntry::sortByName(const DirEntry &i, const DirEntry &j) { return SortByCaseInsensitive(i.name, j.name); }

// ========================================
// Text File I/O (existing)
// ========================================

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

// ========================================
// File Operations
// ========================================

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
    ifstream infile;
    ofstream outfile;

    char *buffer = new char[FILE_BUFFER_SIZE];

    infile.open(source, ios::binary);
    outfile.open(dest, ios::binary);

    if (!infile.good()) {
        delete[] buffer;
        return false;
    }
    if (!outfile.good()) {
        delete[] buffer;
        return false;
    }

    while (true) {
        int read = infile.readsome(buffer, FILE_BUFFER_SIZE);
        if (read == 0)
            break;
        outfile.write(buffer, read);
    }
    infile.close();
    outfile.flush();
    outfile.close();
    delete[] buffer;

    return true;
}

bool copyFile(const string &pathFrom, const string &pathTo) { return copy(pathFrom, pathTo); }

bool removeFile(const string &path) { return remove(path.c_str()) == 0; }

bool renameFile(const string &pathFrom, const string &pathTo) { return rename(pathFrom.c_str(), pathTo.c_str()) == 0; }

// ========================================
// Directory Operations
// ========================================

bool createDir(const string &_name) {
    auto name = fixPath(_name);
    const int dir_err = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return (-1 != dir_err);
}

int rmDir(string path) {
    path = fixPath(path);
    DIR *d = opendir(path.c_str());
    size_t path_len = path.size();
    int r = -1;

    if (d) {
        struct dirent *p;
        r = 0;

        while (!r && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = new char[len];

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path.c_str(), p->d_name);

                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        r2 = rmDir(buf);
                    } else {
                        r2 = unlink(buf);
                    }
                }
                delete[] buf;
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r) {
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

// ========================================
// Directory Listing
// ========================================

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

// ========================================
// Path Manipulation (delegates to PathUtils)
// ========================================

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

// ========================================
// Extension Handling
// ========================================

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
    if (path.length() < 4) {
        return false;
    }
    string fileExt = path.substr(path.length() - 4, path.length());
    if (fileExt[0] != '.') {
        return false;
    }
    auto temp = addDotToExtension(ext);
    return lcase(fileExt) == lcase(temp);
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

// ========================================
// File Content Operations
// ========================================

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

// ========================================
// Utility Functions
// ========================================

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

// ========================================
// Game-Specific Operations (PS1)
// ========================================

bool isPBPFile(string path) {
    if (path.length() < 4)
        return false;
    string last_four = path.substr(path.length() - 4);
    lcase(last_four);
    return last_four == ".pbp";
}

void generateM3UForDirectory(string path, string basename) {
    if (isPBPFile(basename)) {
        basename = basename.substr(0, basename.length() - 4);
    }
    vector<string> files;
    DirEntries filesInPath = diru_FilesOnly(path);
    for (const DirEntry &entry : filesInPath) {
        string ext = getFileExtension(entry.name);
        if (StringUtils::compareCaseInsensitive(ext, "pbp") || StringUtils::compareCaseInsensitive(ext, "cue"))
            files.push_back(entry.name);
    }
    string m3uName = fixPath(path) + sep + basename + ".m3u";
    if (files.size() > 1) {
        ofstream os(m3uName);
        for (const string &file : files) {
            os << file << endl;
        }
        os.close();
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
