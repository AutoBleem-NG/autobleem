// System process utilities
// Original author: screemer
#pragma once

#include <string>
#include <vector>

namespace System {

// Execute a shell command and capture output (newlines stripped).
// Returns empty string if cmd is nullptr.
// Throws std::runtime_error if popen() fails.
std::string executeCommand(const char *cmd);

// Execute a program via fork/exec pattern, waiting for completion.
// Does nothing if cmd is nullptr or args is empty.
// Args must be null-terminated (last element should be nullptr).
void executeFork(const char *cmd, const std::vector<const char *> &args);

// Platform-specific shutdown (exits on x86, shuts down on ARM).
// Creates /tmp/.autobleem_exit signal file before exiting.
[[noreturn]] void shutdown();

} // namespace System
