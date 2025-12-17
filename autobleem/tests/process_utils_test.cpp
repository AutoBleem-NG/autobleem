// Unit tests for System::ProcessUtils
#include <gtest/gtest.h>
#include <string>

#include "../code/system/process_utils.h"

// ============================================================================
// executeCommand() Tests
// ============================================================================

TEST(ProcessUtilsTest, ExecuteCommandEcho) {
    std::string result = System::executeCommand("echo 'test'");
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("test"), std::string::npos);
}

TEST(ProcessUtilsTest, ExecuteCommandPwd) {
    std::string result = System::executeCommand("pwd");
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("/"), std::string::npos);
}

TEST(ProcessUtilsTest, ExecuteCommandWithPipe) {
    std::string result = System::executeCommand("echo 'hello world' | wc -w");
    EXPECT_FALSE(result.empty());
}

TEST(ProcessUtilsTest, ExecuteCommandLs) {
    std::string result = System::executeCommand("ls /tmp");
    // Should execute without crashing, may or may not have output
}

TEST(ProcessUtilsTest, ExecuteCommandEnvVariable) {
    std::string result = System::executeCommand("echo $HOME");
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("/"), std::string::npos);
}

// ============================================================================
// executeFork() Tests
// ============================================================================

TEST(ProcessUtilsTest, ExecuteForkEcho) {
    std::vector<const char*> args = {"echo", "test", nullptr};
    // Should execute without crashing
    System::executeFork("echo", args);
}

TEST(ProcessUtilsTest, ExecuteForkTrue) {
    std::vector<const char*> args = {"true", nullptr};
    System::executeFork("true", args);
}
