// Unit tests for logging functionality
//
// Tests the custom Log::Formatter and log message capture using plog's
// custom appender mechanism.

#include <gtest/gtest.h>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Appenders/IAppender.h>
#include <regex>
#include <string>
#include <vector>

// Include our custom formatter
#include "log.h"

// Custom appender to capture log messages for testing
class TestAppender : public plog::IAppender {
  public:
    void write(const plog::Record &record) override {
        messages.push_back(Log::Formatter::format(record));
    }

    std::vector<std::string> messages;

    void clear() { messages.clear(); }

    bool contains(const std::string &substr) {
        for (const auto &msg : messages) {
            if (msg.find(substr) != std::string::npos)
                return true;
        }
        return false;
    }
};

// Global test appender - initialized once for all tests
static TestAppender *testAppender = nullptr;

class LogTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite() {
        testAppender = new TestAppender();
        plog::init(plog::debug, testAppender);
    }

    void SetUp() override { testAppender->clear(); }
};

//=============================================================================
// Formatter Tests
//=============================================================================

TEST_F(LogTest, FormatterProducesExpectedFormat) {
    PLOG_INFO << "test message";

    ASSERT_EQ(1, testAppender->messages.size());
    std::string msg = testAppender->messages[0];

    // Expected format: "HH:MM:SS INFO  [FunctionName:LineNum] test message\n"
    // Example: "14:32:01 INFO  [TestBody:52] test message\n"
    std::regex pattern(R"(\d{2}:\d{2}:\d{2} INFO  \[.*:\d+\] test message\n)");
    EXPECT_TRUE(std::regex_match(msg, pattern)) << "Unexpected format: " << msg;
}

TEST_F(LogTest, FormatterIncludesTimestamp) {
    PLOG_INFO << "timestamp test";

    ASSERT_EQ(1, testAppender->messages.size());
    std::string msg = testAppender->messages[0];

    // Check timestamp format HH:MM:SS at start
    std::regex timestampPattern(R"(^\d{2}:\d{2}:\d{2} )");
    EXPECT_TRUE(std::regex_search(msg, timestampPattern)) << "Missing timestamp: " << msg;
}

TEST_F(LogTest, FormatterIncludesFunctionAndLine) {
    PLOG_INFO << "location test";

    ASSERT_EQ(1, testAppender->messages.size());
    std::string msg = testAppender->messages[0];

    // Check for [function:line] format
    std::regex locationPattern(R"(\[.*:\d+\])");
    EXPECT_TRUE(std::regex_search(msg, locationPattern)) << "Missing location: " << msg;
}

//=============================================================================
// Severity Level Tests
//=============================================================================

TEST_F(LogTest, DebugSeverity) {
    PLOG_DEBUG << "debug message";

    ASSERT_EQ(1, testAppender->messages.size());
    EXPECT_TRUE(testAppender->contains("DEBUG")) << testAppender->messages[0];
}

TEST_F(LogTest, InfoSeverity) {
    PLOG_INFO << "info message";

    ASSERT_EQ(1, testAppender->messages.size());
    EXPECT_TRUE(testAppender->contains("INFO")) << testAppender->messages[0];
}

TEST_F(LogTest, WarningSeverity) {
    PLOG_WARNING << "warning message";

    ASSERT_EQ(1, testAppender->messages.size());
    EXPECT_TRUE(testAppender->contains("WARN")) << testAppender->messages[0];
}

TEST_F(LogTest, ErrorSeverity) {
    PLOG_ERROR << "error message";

    ASSERT_EQ(1, testAppender->messages.size());
    EXPECT_TRUE(testAppender->contains("ERROR")) << testAppender->messages[0];
}

TEST_F(LogTest, AllSeverityLevelsLogged) {
    PLOG_DEBUG << "debug";
    PLOG_INFO << "info";
    PLOG_WARNING << "warning";
    PLOG_ERROR << "error";

    EXPECT_EQ(4, testAppender->messages.size());
}

//=============================================================================
// Message Content Tests
//=============================================================================

TEST_F(LogTest, StreamOperatorWithInt) {
    int value = 42;
    PLOG_INFO << "Value: " << value;

    EXPECT_TRUE(testAppender->contains("Value: 42"));
}

TEST_F(LogTest, StreamOperatorWithString) {
    std::string name = "AutoBleem";
    PLOG_INFO << "App: " << name;

    EXPECT_TRUE(testAppender->contains("App: AutoBleem"));
}

TEST_F(LogTest, StreamOperatorChained) {
    PLOG_INFO << "a=" << 1 << " b=" << 2 << " c=" << 3;

    EXPECT_TRUE(testAppender->contains("a=1 b=2 c=3"));
}
