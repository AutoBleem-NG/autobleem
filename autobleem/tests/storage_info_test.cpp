// Unit tests for System::StorageInfo
#include <gtest/gtest.h>

#include "../code/system/storage_info.h"

// ============================================================================
// StorageInfo struct Tests
// ============================================================================

TEST(StorageInfoTest, FormattedOutput) {
    System::StorageInfo info;
    info.freeGB = 100.5f;
    info.totalGB = 500.0f;
    info.freePercent = 20;

    std::string result = info.formatted();
    EXPECT_FALSE(result.empty());
    // Should contain "100.50 GB / 500.00 GB (20%)"
    EXPECT_NE(result.find("GB"), std::string::npos);
    EXPECT_NE(result.find("%"), std::string::npos);
}

TEST(StorageInfoTest, FormattedZeroValues) {
    System::StorageInfo info{0.0f, 0.0f, 0};
    std::string result = info.formatted();
    EXPECT_FALSE(result.empty());
}

// ============================================================================
// getStorageInfo() Tests
// ============================================================================

TEST(StorageInfoTest, GetStorageInfoRoot) {
    // Test getting storage info for root path
    System::StorageInfo info = System::getStorageInfo("/");

    // On x86, returns zeros; on ARM, returns actual values
    EXPECT_GE(info.freePercent, 0);
    EXPECT_LE(info.freePercent, 100);
    EXPECT_LE(info.freeGB, info.totalGB);
}

TEST(StorageInfoTest, GetStorageInfoTmp) {
    System::StorageInfo info = System::getStorageInfo("/tmp");

    EXPECT_GE(info.freePercent, 0);
    EXPECT_LE(info.freePercent, 100);
}

TEST(StorageInfoTest, GetStorageInfoConsistency) {
    // Multiple calls should return consistent results
    System::StorageInfo info1 = System::getStorageInfo("/");
    System::StorageInfo info2 = System::getStorageInfo("/");

    // Total should be the same
    EXPECT_EQ(info1.totalGB, info2.totalGB);
}
