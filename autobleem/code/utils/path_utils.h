#pragma once

#include <string>

// Unified path utilities - consolidates path operations from DirEntry, util, and environment

namespace PathUtils {

// Get the file extension from a path WITHOUT the dot (e.g., "bin" from "/path/to/file.bin")
// Returns empty string if no extension found
std::string getFileExtension(const std::string &path);

// Get filename without extension (e.g., "file" from "/path/to/file.bin")
std::string getFileNameWithoutExtension(const std::string &path);

// Get filename from path (e.g., "file.bin" from "/path/to/file.bin")
std::string getFileNameFromPath(const std::string &path);

// Get directory path from full path (e.g., "/path/to" from "/path/to/file.bin")
// Does NOT include trailing separator
std::string getDirNameFromPath(const std::string &path);

// Join directory and filename with appropriate separator
std::string joinPath(const std::string &dir, const std::string &file);

// Remove trailing separator from path if present
std::string removeSeparatorFromEndOfPath(const std::string &path);

// Normalize path: remove leading/trailing spaces and trailing separator
std::string fixPath(const std::string &path);

// Platform-specific separator ('/' on Unix, '\\' on Windows)
extern const char separator;

} // namespace PathUtils
