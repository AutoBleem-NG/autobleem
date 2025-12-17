//
// System process utilities
// Extracted from util.cpp
// Original author: screemer
//
#pragma once

#include <string>
#include <vector>

namespace System {

// Execute a shell command and capture output
std::string executeCommand(const char *cmd);

// Execute a program via fork/exec pattern
void executeFork(const char *cmd, const std::vector<const char *> &args);

// Platform-specific shutdown (exits on x86, shuts down on ARM)
void shutdown();

} // namespace System
