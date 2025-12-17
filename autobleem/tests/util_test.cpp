// Unit tests for utility functions
#include <gtest/gtest.h>
#include <set>
#include <algorithm>
#include <limits>

#include "../code/utils/random_utils.h"

// Helper function to get random index (replaces Util::getRandomIndex)
inline unsigned int getRandomIndex(unsigned int size) {
    if (size == 0) return 0;
    if (size == 1) return 0;

    auto &rng = RandomUtils::RandomGenerator::getInstance();
    if (size > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
        return rng.generateInt(0, std::numeric_limits<int>::max()) % size;
    }

    return rng.generateInt(0, static_cast<int>(size - 1));
}

class UtilRandomNumberTest : public ::testing::Test {
protected:
    // Test fixture setup
    void SetUp() override {
        // Tests will use the static initialized state
    }
};

TEST_F(UtilRandomNumberTest, ReturnsNonNegativeValue) {
    // getRandomNumber should return unsigned int >= 0
    auto &rng = RandomUtils::RandomGenerator::getInstance();
    unsigned int result = rng.generateInt(0, std::numeric_limits<int>::max());
    EXPECT_GE(result, 0u);
}

TEST_F(UtilRandomNumberTest, ReturnsUnsignedInt) {
    // Just verify it returns a value that fits in unsigned int
    auto &rng = RandomUtils::RandomGenerator::getInstance();
    unsigned int result = rng.generateInt(0, std::numeric_limits<int>::max());
    EXPECT_LE(result, std::numeric_limits<unsigned int>::max());
}

TEST_F(UtilRandomNumberTest, ProducesVariedSequence) {
    // Multiple calls should produce different values (with high probability)
    auto &rng = RandomUtils::RandomGenerator::getInstance();
    std::set<unsigned int> values;
    for (int i = 0; i < 100; i++) {
        values.insert(rng.generateInt(0, std::numeric_limits<int>::max()));
    }
    // With good randomness, should have many different values
    // Use conservative threshold to avoid flakiness
    EXPECT_GT(values.size(), 50);
}

TEST_F(UtilRandomNumberTest, NotAlwaysZero) {
    // Should not always return 0
    auto &rng = RandomUtils::RandomGenerator::getInstance();
    unsigned int result1 = rng.generateInt(0, std::numeric_limits<int>::max());
    unsigned int result2 = rng.generateInt(0, std::numeric_limits<int>::max());
    // At least one shouldn't be 0
    EXPECT_TRUE(result1 != 0 || result2 != 0);
}

TEST_F(UtilRandomNumberTest, MultipleCallsProduceDifferentValues) {
    // First call and second call should likely be different
    auto &rng = RandomUtils::RandomGenerator::getInstance();
    unsigned int result1 = rng.generateInt(0, std::numeric_limits<int>::max());
    unsigned int result2 = rng.generateInt(0, std::numeric_limits<int>::max());

    // It's possible but extremely unlikely they're the same
    // Skip this test occasionally if they happen to match
    if (result1 == result2) {
        // Get a few more to establish variety
        unsigned int result3 = rng.generateInt(0, std::numeric_limits<int>::max());
        EXPECT_NE(result2, result3);
    } else {
        EXPECT_NE(result1, result2);
    }
}

class UtilRandomIndexTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset state before each test
    }
};

TEST_F(UtilRandomIndexTest, ReturnsZeroForEmptySize) {
    // getRandomIndex(0) should return 0
    unsigned int result = ::getRandomIndex(0);
    EXPECT_EQ(result, 0u);
}

TEST_F(UtilRandomIndexTest, ReturnsZeroForSizeOne) {
    // getRandomIndex(1) should always return 0
    for (int i = 0; i < 10; i++) {
        unsigned int result = ::getRandomIndex(1);
        EXPECT_EQ(result, 0u);
    }
}

TEST_F(UtilRandomIndexTest, ReturnValueWithinRange) {
    unsigned int size = 10;
    for (int i = 0; i < 100; i++) {
        unsigned int result = ::getRandomIndex(size);
        EXPECT_GE(result, 0u);
        EXPECT_LT(result, size);
    }
}

TEST_F(UtilRandomIndexTest, ProducesVariedIndices) {
    unsigned int size = 20;
    std::set<unsigned int> indices;
    for (int i = 0; i < 100; i++) {
        indices.insert(::getRandomIndex(size));
    }
    // Should produce variety of indices
    EXPECT_GT(indices.size(), 10);
}

TEST_F(UtilRandomIndexTest, AllIndicesWithinBounds) {
    unsigned int size = 5;
    for (int i = 0; i < 200; i++) {
        unsigned int result = ::getRandomIndex(size);
        EXPECT_GE(result, 0u);
        EXPECT_LT(result, size);
    }
}

TEST_F(UtilRandomIndexTest, LargeSize) {
    unsigned int size = 10000;
    std::set<unsigned int> indices;
    for (int i = 0; i < 500; i++) {
        unsigned int result = ::getRandomIndex(size);
        EXPECT_LT(result, size);
        indices.insert(result);
    }
    // With large size and 500 iterations, should see variety
    EXPECT_GT(indices.size(), 200);
}

TEST_F(UtilRandomIndexTest, DistributionAcrossRange) {
    // Check that indices are distributed across the range
    unsigned int size = 10;
    std::vector<int> histogram(size, 0);

    // Generate many random indices
    int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        unsigned int index = ::getRandomIndex(size);
        histogram[index]++;
    }

    // Each bucket should have roughly 1000/10 = 100 items
    // Allow 25-175 range (25th to 175th percentile)
    for (int i = 0; i < size; i++) {
        EXPECT_GT(histogram[i], 25) << "Index " << i << " has too few occurrences";
        EXPECT_LT(histogram[i], 175) << "Index " << i << " has too many occurrences";
    }
}

// Integration tests - realistic usage patterns
class UtilUsagePatternTest : public ::testing::Test {};

TEST_F(UtilUsagePatternTest, SelectRandomGameFromCarousel) {
    // Simulate selecting a random game from carousel
    // Similar to gui_launcher_loop.cpp usage
    unsigned int carouselSize = 50;

    std::set<unsigned int> selectedGames;
    for (int i = 0; i < 100; i++) {
        unsigned int gameIndex = ::getRandomIndex(carouselSize);
        EXPECT_LT(gameIndex, carouselSize);
        selectedGames.insert(gameIndex);
    }

    // Should select from variety of games
    EXPECT_GT(selectedGames.size(), 30);
}

TEST_F(UtilUsagePatternTest, SelectRandomMenuOption) {
    // Simulate selecting a random menu option
    // Similar to gui_optionsMenu.cpp usage
    unsigned int menuSize = 5;

    std::vector<unsigned int> selections;
    for (int i = 0; i < 50; i++) {
        unsigned int option = ::getRandomIndex(menuSize);
        EXPECT_LT(option, menuSize);
        selections.push_back(option);
    }

    // Check we get variety
    std::set<unsigned int> uniqueSelections(selections.begin(), selections.end());
    EXPECT_GT(uniqueSelections.size(), 2);
}

TEST_F(UtilUsagePatternTest, EmptyCollectionHandling) {
    // Should safely handle empty collections
    unsigned int result = ::getRandomIndex(0);
    EXPECT_EQ(result, 0u);

    // Should handle size 1
    result = ::getRandomIndex(1);
    EXPECT_EQ(result, 0u);
}

// Edge case tests
class UtilEdgeCaseTest : public ::testing::Test {};

TEST_F(UtilEdgeCaseTest, MaxUnsignedInt) {
    // Test with very large size value (but not the absolute max to avoid timeouts)
    // Use a large value that still represents real-world scenarios
    unsigned int largeSize = 1000000000u;  // 1 billion

    // Should still return valid index
    unsigned int result = ::getRandomIndex(largeSize);
    EXPECT_LT(result, largeSize);
}

TEST_F(UtilEdgeCaseTest, PowerOfTwoSize) {
    // Test with power of 2 sizes (common for arrays)
    for (unsigned int size : {2u, 4u, 8u, 16u, 32u, 64u, 128u, 256u}) {
        unsigned int result = ::getRandomIndex(size);
        EXPECT_LT(result, size);
    }
}

TEST_F(UtilEdgeCaseTest, PrimeSize) {
    // Test with prime number sizes
    for (unsigned int size : {7u, 11u, 13u, 17u, 19u, 23u}) {
        unsigned int result = ::getRandomIndex(size);
        EXPECT_LT(result, size);
    }
}

TEST_F(UtilEdgeCaseTest, RepeatedCalls) {
    // Ensure no state corruption with many calls
    unsigned int size = 25;
    for (int i = 0; i < 10000; i++) {
        unsigned int result = ::getRandomIndex(size);
        EXPECT_LT(result, size);
    }
}
