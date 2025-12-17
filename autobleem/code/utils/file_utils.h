#pragma once

#include <string>
#include <vector>

// Unified file I/O utilities - consolidates file operations from util and throughout codebase

namespace FileUtils {

// Read entire text file into vector of strings, one line per element
// Optional: remove CRLF line endings if specified
std::vector<std::string> readTextFile(const std::string &filePath, bool removeCRLF = true);

// Write vector of strings to text file, one string per line
// Optional: append line ending after each string
bool writeTextFile(const std::string &filePath, const std::vector<std::string> &lines, bool appendLineEnding = true);

// Read getline from stream with optional CR removal
std::istream &getlineRemoveCR(std::istream &is, std::string &str);

} // namespace FileUtils
