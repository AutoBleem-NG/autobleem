// System process utilities
// Original author: screemer
#include "process_utils.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace System {

void shutdown() {
    // Signal boot.sh loop to exit (in case shutdown command fails or on dev machine)
    FILE *f = fopen("/tmp/.autobleem_exit", "w");
    if (f != nullptr) {
        fclose(f);
    }

#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    exit(0);
#else
    executeCommand("shutdown -h now");
    exit(0);
#endif
}

std::string executeCommand(const char *cmd) {
    if (cmd == nullptr) {
        return "";
    }

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // Strip all newlines from output
    if (!result.empty()) {
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    }
    return result;
}

void executeFork(const char *cmd, const std::vector<const char *> &args) {
    if (cmd == nullptr || args.empty()) {
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        // fork() failed
        return;
    }

    if (pid == 0) {
        // Child process
        execvp(cmd, const_cast<char **>(args.data()));
        // execvp only returns on error
        _exit(127);
    }

    // Parent process - wait for child
    waitpid(pid, nullptr, 0);
}

} // namespace System
