#pragma once

#include <cstdint>
#include <string>

// RAII timer for measuring code execution time in debug builds.
// Create a DebugTimer at the start of a scope; when it goes out of scope,
// it logs the elapsed time. In release builds, this is a no-op.
#ifndef NDEBUG
struct DebugTimer {
    std::string description;
    uint32_t ticks_start = 0;
    uint32_t ticks_end = 0;

    explicit DebugTimer(const std::string &desc);
    ~DebugTimer();
};
#else
struct DebugTimer {
    explicit DebugTimer(const std::string & = "") {}
};
#endif
