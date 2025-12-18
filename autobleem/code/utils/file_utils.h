#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "../main.h"
#include "path_utils.h"

// Unified file and filesystem utilities
// Consolidates file operations from DirEntry, util, and throughout codebase

namespace FileUtils {

// ========================================
// Directory Entry Structure
// ========================================

struct DirEntry {
    std::string name;
    bool isDir;

    DirEntry(std::string _name, bool dir) : name(_name), isDir(dir) {}

    void print() const;
    static bool sortByName(const DirEntry &i, const DirEntry &j);
};

using DirEntries = std::vector<DirEntry>;

// ========================================
// Text File I/O
// ========================================

std::vector<std::string> readTextFile(const std::string &filePath, bool removeCRLF = true);
bool writeTextFile(const std::string &filePath, const std::vector<std::string> &lines, bool appendLineEnding = true);
std::istream &getlineRemoveCR(std::istream &is, std::string &str);

// ========================================
// File Operations
// ========================================

bool exists(const std::string &path);
bool copy(const std::string &source, const std::string &dest);
bool copyFile(const std::string &pathFrom, const std::string &pathTo);
bool removeFile(const std::string &path);
bool renameFile(const std::string &pathFrom, const std::string &pathTo);
bool isDirectory(const std::string &path);

// ========================================
// Directory Operations
// ========================================

bool createDir(const std::string &name);
int rmDir(std::string path);
bool removeDirAndContents(const std::string &path);

// ========================================
// Directory Listing
// ========================================

DirEntries dir(std::string path);
DirEntries diru(std::string path);
DirEntries diru_DirsOnly(std::string path);
DirEntries diru_FilesOnly(std::string path);

// ========================================
// Path Manipulation (delegates to PathUtils)
// ========================================

std::string fixPath(std::string path);
std::string removeSeparatorFromEndOfPath(const std::string &path);
std::string getFileNameFromPath(const std::string &path);
std::string getDirNameFromPath(const std::string &path);
std::string getFileExtension(const std::string &fileName);
std::string getFileNameWithoutExtension(const std::string &filename);
std::string removeGamesPathFromFrontOfPath(const std::string &path);

// ========================================
// Extension Handling
// ========================================

std::string removeDotFromExtension(const std::string &ext);
std::string addDotToExtension(const std::string &ext);
bool matchExtension(std::string path, std::string ext);
DirEntries getFilesWithExtension(const std::string &path, const DirEntries &entries,
                                 const std::vector<std::string> &extensions);
std::string findFirstFile(std::string ext, std::string path);

// ========================================
// File Content Operations
// ========================================

std::vector<std::string> cueToBinList(std::string cueFile);

// ========================================
// Utility Functions
// ========================================

std::string replaceTheseCharsWithThisChar(std::string str, const std::string &charsToReplace, char replacementChar);
bool fixCommaInDirOrFileName(const std::string &path, DirEntry *entry);

// ========================================
// Game-Specific Operations (PS1)
// ========================================

bool isPBPFile(std::string path);
void generateM3UForDirectory(std::string path, std::string basename);
bool isAGameFile(const std::string &filename);
ImageType getGameFileImageType(const std::string &filename);
bool imageTypeUsesACueFile(ImageType imageType);
bool thereIsAGameFile(const DirEntries &entries);
bool thereIsAGameFile(const std::string &dirpath);
bool thereIsASubDir(const DirEntries &entries);
bool thereIsASubDir(const std::string &dirpath);
std::tuple<ImageType, std::string> getGameFile(const DirEntries &entries);
std::tuple<ImageType, std::string> getGameFile(const std::string &dirpath);

} // namespace FileUtils

// ========================================
// Separator Helper (for path construction)
// ========================================

struct Sep {};
extern const Sep sep;
std::string operator+(const std::string &leftside, Sep);
void operator+=(std::string &leftside, Sep);

// ========================================
// Convenience aliases (avoid FileUtils:: prefix)
// ========================================

using DirEntry = FileUtils::DirEntry;
using DirEntries = FileUtils::DirEntries;
