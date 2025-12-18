// Unit tests for System::ProcessUtils
#include <gtest/gtest.h>
#include <string>

#include "../code/system/process_utils.h"

// executeCommand() Tests

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

TEST(ProcessUtilsTest, ExecuteCommandNullptr) {
    std::string result = System::executeCommand(nullptr);
    EXPECT_TRUE(result.empty());
}

TEST(ProcessUtilsTest, ExecuteCommandEmptyOutput) {
    // true command produces no output
    std::string result = System::executeCommand("true");
    EXPECT_TRUE(result.empty());
}

TEST(ProcessUtilsTest, ExecuteCommandStripsNewlines) {
    // Multi-line output should have newlines stripped
    std::string result = System::executeCommand("printf 'line1\\nline2'");
    EXPECT_EQ(result.find('\n'), std::string::npos);
    EXPECT_NE(result.find("line1"), std::string::npos);
    EXPECT_NE(result.find("line2"), std::string::npos);
}

// executeFork() Tests

TEST(ProcessUtilsTest, ExecuteForkEcho) {
    std::vector<const char*> args = {"echo", "test", nullptr};
    // Should execute without crashing
    System::executeFork("echo", args);
}

TEST(ProcessUtilsTest, ExecuteForkTrue) {
    std::vector<const char*> args = {"true", nullptr};
    System::executeFork("true", args);
}

TEST(ProcessUtilsTest, ExecuteForkNullCmd) {
    std::vector<const char*> args = {"echo", nullptr};
    // Should not crash with nullptr cmd
    System::executeFork(nullptr, args);
}

TEST(ProcessUtilsTest, ExecuteForkEmptyArgs) {
    std::vector<const char*> args;
    // Should not crash with empty args
    System::executeFork("echo", args);
}

TEST(ProcessUtilsTest, ExecuteForkInvalidCommand) {
    std::vector<const char*> args = {"nonexistent_command_xyz", nullptr};
    // Should not crash, execvp will fail and child exits with 127
    System::executeFork("nonexistent_command_xyz", args);
}
