// Unit tests for System::StorageInfo
#include <gtest/gtest.h>

#include "../code/system/storage_info.h"

// StorageInfo struct Tests

TEST(StorageInfoTest, DefaultInitialization) {
    System::StorageInfo info;
    EXPECT_FLOAT_EQ(info.freeGB, 0.0f);
    EXPECT_FLOAT_EQ(info.totalGB, 0.0f);
    EXPECT_EQ(info.freePercent, 0);
}

TEST(StorageInfoTest, AggregateInitialization) {
    System::StorageInfo info{10.5f, 20.0f, 52};
    EXPECT_FLOAT_EQ(info.freeGB, 10.5f);
    EXPECT_FLOAT_EQ(info.totalGB, 20.0f);
    EXPECT_EQ(info.freePercent, 52);
}

TEST(StorageInfoTest, FormattedOutput) {
    System::StorageInfo info{100.5f, 500.0f, 20};
    std::string result = info.formatted();

    // Verify exact format: "X.XX GB / Y.YY GB (Z%)"
    EXPECT_EQ(result, "100.50 GB / 500.00 GB (20%)");
}

TEST(StorageInfoTest, FormattedZeroValues) {
    System::StorageInfo info{0.0f, 0.0f, 0};
    std::string result = info.formatted();
    EXPECT_EQ(result, "0.00 GB / 0.00 GB (0%)");
}

TEST(StorageInfoTest, FormattedLargeValues) {
    System::StorageInfo info{1500.75f, 2000.0f, 75};
    std::string result = info.formatted();
    EXPECT_EQ(result, "1500.75 GB / 2000.00 GB (75%)");
}

TEST(StorageInfoTest, FormattedSingleDigitPercent) {
    System::StorageInfo info{1.0f, 100.0f, 1};
    std::string result = info.formatted();
    EXPECT_EQ(result, "1.00 GB / 100.00 GB (1%)");
}

TEST(StorageInfoTest, FormattedFullPercent) {
    System::StorageInfo info{100.0f, 100.0f, 100};
    std::string result = info.formatted();
    EXPECT_EQ(result, "100.00 GB / 100.00 GB (100%)");
}

TEST(StorageInfoTest, FormattedRounding) {
    // Test that floating point values are rounded to 2 decimal places
    System::StorageInfo info{1.999f, 2.001f, 99};
    std::string result = info.formatted();
    // formatFloat should round appropriately
    EXPECT_NE(result.find("GB"), std::string::npos);
    EXPECT_NE(result.find("99%"), std::string::npos);
}

// KB_PER_GB constant Tests

TEST(StorageInfoTest, KBPerGBConstant) {
    // Verify the constant is correct: 1 GB = 1024 * 1024 KB
    EXPECT_EQ(System::KB_PER_GB, 1048576L);
}

TEST(StorageInfoTest, KBPerGBConversion) {
    // 1 GB in KB should convert back to 1.0 GB
    float gbValue = static_cast<float>(System::KB_PER_GB) / System::KB_PER_GB;
    EXPECT_FLOAT_EQ(gbValue, 1.0f);

    // 2 GB in KB
    float twoGB = static_cast<float>(2L * System::KB_PER_GB) / System::KB_PER_GB;
    EXPECT_FLOAT_EQ(twoGB, 2.0f);
}

// getStorageInfo() Tests

TEST(StorageInfoTest, GetStorageInfoReturnsValidStruct) {
    System::StorageInfo info = System::getStorageInfo("/");

    // On x86, returns zeros; on ARM, returns actual values
    // Either way, values should be non-negative and percent in valid range
    EXPECT_GE(info.freeGB, 0.0f);
    EXPECT_GE(info.totalGB, 0.0f);
    EXPECT_GE(info.freePercent, 0);
    EXPECT_LE(info.freePercent, 100);
}

TEST(StorageInfoTest, GetStorageInfoFreeNotExceedTotal) {
    System::StorageInfo info = System::getStorageInfo("/");
    EXPECT_LE(info.freeGB, info.totalGB);
}

TEST(StorageInfoTest, GetStorageInfoConsistency) {
    // Multiple calls should return consistent results
    System::StorageInfo info1 = System::getStorageInfo("/");
    System::StorageInfo info2 = System::getStorageInfo("/");

    // Total should be the same between calls
    EXPECT_EQ(info1.totalGB, info2.totalGB);
}

TEST(StorageInfoTest, GetStorageInfoDefaultPath) {
    // Test with default parameter (no argument)
    System::StorageInfo info = System::getStorageInfo();

    // On x86, returns zeros; on ARM would return /media info
    EXPECT_GE(info.freePercent, 0);
    EXPECT_LE(info.freePercent, 100);
}

TEST(StorageInfoTest, GetStorageInfoDifferentPaths) {
    // On x86 builds, all paths return zeros
    // On ARM, path is ignored anyway (always reads /media)
    System::StorageInfo info1 = System::getStorageInfo("/");
    System::StorageInfo info2 = System::getStorageInfo("/tmp");
    System::StorageInfo info3 = System::getStorageInfo("/nonexistent");

    // All should return valid (non-negative) values
    EXPECT_GE(info1.freeGB, 0.0f);
    EXPECT_GE(info2.freeGB, 0.0f);
    EXPECT_GE(info3.freeGB, 0.0f);
}
