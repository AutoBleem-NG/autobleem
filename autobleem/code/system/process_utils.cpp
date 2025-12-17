//
// System process utilities
// Extracted from util.cpp
// Original author: screemer
//
#include "process_utils.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace System {

void shutdown() {
#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    exit(0);
#else
    executeCommand("shutdown -h now");
    exit(0);
#endif
}

string executeCommand(const char *cmd) {
    array<char, 128> buffer;
    string result;

    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (!result.empty()) {
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
    }
    return result;
}

void executeFork(const char *cmd, const vector<const char *> &args) {
    string link = cmd;

    int pid = fork();
    if (!pid) {
        execvp(link.c_str(), const_cast<char **>(args.data()));
    }

    waitpid(pid, nullptr, 0);
}

} // namespace System
