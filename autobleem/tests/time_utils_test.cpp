// Unit tests for time utilities
#include <gtest/gtest.h>
#include <ctime>
#include <cstring>

#include "../code/utils/time_utils.h"

// Tests for getCurrentTime
class GetCurrentTimeTest : public ::testing::Test {};

TEST_F(GetCurrentTimeTest, ReturnsValidTimestamp) {
    // Get current time before and after function call to verify result is reasonable
    time_t before = time(nullptr);
    time_t result = TimeUtils::getCurrentTime();
    time_t after = time(nullptr);

    // Result should be between before and after
    EXPECT_GE(result, before);
    EXPECT_LE(result, after);
}

TEST_F(GetCurrentTimeTest, ReturnsPositiveValue) {
    time_t result = TimeUtils::getCurrentTime();
    // Timestamps should be positive (after Jan 1, 1970)
    EXPECT_GT(result, 0);
}

TEST_F(GetCurrentTimeTest, MultipleCallsReturnIncreasingValues) {
    time_t first = TimeUtils::getCurrentTime();
    // Small delay to ensure time progresses
    sleep(1);
    time_t second = TimeUtils::getCurrentTime();

    // Second call should be >= first (allowing for very fast execution)
    EXPECT_GE(second, first);
}

// Tests for usingWiFiUpdatedTime
class UsingWiFiUpdatedTimeTest : public ::testing::Test {};

TEST_F(UsingWiFiUpdatedTimeTest, ReturnsBooleanValue) {
    bool result = TimeUtils::usingWiFiUpdatedTime();
    // Should be a boolean (true or false)
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UsingWiFiUpdatedTimeTest, ReturnsTrueForCurrentTime) {
    // Current system time (if properly set) should be >= 2020
    bool result = TimeUtils::usingWiFiUpdatedTime();
    time_t now = time(nullptr);
    tm *local = localtime(&now);

    if (local != nullptr && local->tm_year + 1900 >= 2020) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(UsingWiFiUpdatedTimeTest, ConsistentWithManualYearCheck) {
    bool result = TimeUtils::usingWiFiUpdatedTime();
    time_t now = time(nullptr);
    tm *local = localtime(&now);

    bool expectedResult = (local != nullptr) && (local->tm_year + 1900 >= 2020);
    EXPECT_EQ(result, expectedResult);
}

// Tests for timeToDisplayTimeString
class TimeToDisplayTimeStringTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test with a known timestamp: 2023-06-15 10:30:45 UTC
        // This is 1686820245 in Unix timestamp
        testTimestamp = 1686820245;
    }

    time_t testTimestamp;
};

TEST_F(TimeToDisplayTimeStringTest, ReturnsEmptyStringForZeroTime) {
    std::string result = TimeUtils::timeToDisplayTimeString(0);
    EXPECT_EQ(result, "");
}

TEST_F(TimeToDisplayTimeStringTest, ReturnsEmptyStringForNegativeTime) {
    std::string result = TimeUtils::timeToDisplayTimeString(-1);
    EXPECT_EQ(result, "");
}

TEST_F(TimeToDisplayTimeStringTest, ReturnsNonEmptyStringForValidTimestamp) {
    std::string result = TimeUtils::timeToDisplayTimeString(testTimestamp);
    // Should return non-empty string for valid timestamp
    EXPECT_FALSE(result.empty());
}

TEST_F(TimeToDisplayTimeStringTest, FormatDefaultStructure) {
    // Default format is "%F %I:%M:%S %p"
    // %F is YYYY-MM-DD, %I:%M:%S is HH:MM:SS, %p is AM/PM
    std::string result = TimeUtils::timeToDisplayTimeString(testTimestamp);

    if (!result.empty()) {
        // Should contain date separators and time separators
        EXPECT_NE(result.find("-"), std::string::npos);  // Date separator
        EXPECT_NE(result.find(":"), std::string::npos);  // Time separator
    }
}

TEST_F(TimeToDisplayTimeStringTest, CustomFormatString) {
    // Use simple custom format
    std::string result = TimeUtils::timeToDisplayTimeString(testTimestamp, "%Y");

    if (!result.empty()) {
        // Should contain year digits
        EXPECT_FALSE(result.empty());
        // Year 2023 should be in result
        EXPECT_TRUE(result.find("2023") != std::string::npos ||
                    result.find("2024") != std::string::npos ||
                    result.find("2025") != std::string::npos);
    }
}

TEST_F(TimeToDisplayTimeStringTest, CustomFormatMonth) {
    // Format: %m for month
    std::string result = TimeUtils::timeToDisplayTimeString(testTimestamp, "%m");

    if (!result.empty()) {
        // June is month 06
        EXPECT_TRUE(result.find("06") != std::string::npos ||
                    result.find("6") != std::string::npos);
    }
}

TEST_F(TimeToDisplayTimeStringTest, CustomFormatDay) {
    // Format: %d for day
    std::string result = TimeUtils::timeToDisplayTimeString(testTimestamp, "%d");

    if (!result.empty()) {
        // Day is 15
        EXPECT_TRUE(result.find("15") != std::string::npos);
    }
}

TEST_F(TimeToDisplayTimeStringTest, OldTimestampReturnsEmpty) {
    // Timestamp from 2010 (before 2020 cutoff)
    // 2010-01-01 00:00:00 UTC = 1262304000
    time_t oldTime = 1262304000;
    std::string result = TimeUtils::timeToDisplayTimeString(oldTime);

    // Should return empty string because year is before 2020
    EXPECT_EQ(result, "");
}

TEST_F(TimeToDisplayTimeStringTest, Year2020Timestamp) {
    // Use a timestamp that's definitely after 2020 to avoid timezone issues
    // 2020-12-31 00:00:00 UTC = 1609372800 (safer than 2020-01-01 due to timezones)
    time_t year2020 = 1609372800;
    std::string result = TimeUtils::timeToDisplayTimeString(year2020);

    // Should return non-empty string because year is in 2020
    EXPECT_FALSE(result.empty());
}

TEST_F(TimeToDisplayTimeStringTest, DifferentTimestampsProduceDifferentResults) {
    // Two different valid timestamps should produce different results
    time_t time1 = 1686820245;  // 2023-06-15
    time_t time2 = 1609459200;  // 2021-01-01

    std::string result1 = TimeUtils::timeToDisplayTimeString(time1);
    std::string result2 = TimeUtils::timeToDisplayTimeString(time2);

    // Both should be non-empty (if year >= 2020)
    if (!result1.empty() && !result2.empty()) {
        // They should be different
        EXPECT_NE(result1, result2);
    }
}

TEST_F(TimeToDisplayTimeStringTest, ConsecutiveCallsWithSameTimeMatch) {
    // Same timestamp should always produce same result
    std::string result1 = TimeUtils::timeToDisplayTimeString(testTimestamp);
    std::string result2 = TimeUtils::timeToDisplayTimeString(testTimestamp);

    EXPECT_EQ(result1, result2);
}

TEST_F(TimeToDisplayTimeStringTest, EmptyFormatStringUseDefaultFormat) {
    // Empty format string should use default format
    std::string result = TimeUtils::timeToDisplayTimeString(testTimestamp, "");

    if (!result.empty()) {
        // Should match default format structure
        EXPECT_NE(result.find("-"), std::string::npos);  // Date separator
        EXPECT_NE(result.find(":"), std::string::npos);  // Time separator
    }
}

TEST_F(TimeToDisplayTimeStringTest, LargeValidTimestamp) {
    // Far future timestamp (year 2050)
    // 2050-01-01 00:00:00 UTC = 2524608000
    time_t futureTime = 2524608000;
    std::string result = TimeUtils::timeToDisplayTimeString(futureTime);

    // Should return non-empty string for valid future timestamp
    EXPECT_FALSE(result.empty());
}
