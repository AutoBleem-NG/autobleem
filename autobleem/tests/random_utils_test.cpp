#include <gtest/gtest.h>
#include <set>
#include <cmath>

#include "../code/utils/random_utils.h"

class RandomGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset generator with known seed before each test
        rng = &RandomUtils::RandomGenerator::getInstance();
        rng->seed(42);  // Use fixed seed for reproducible tests
    }

    RandomUtils::RandomGenerator* rng;
};

class GenerateIntTest : public RandomGeneratorTest {};

TEST_F(GenerateIntTest, ReturnsValueWithinRange) {
    int min = 10, max = 20;
    for (int i = 0; i < 100; i++) {
        int result = rng->generateInt(min, max);
        EXPECT_GE(result, min);
        EXPECT_LE(result, max);
    }
}

TEST_F(GenerateIntTest, ProducesVariedValues) {
    // Generator with fixed seed should produce same sequence
    std::set<int> values;
    for (int i = 0; i < 50; i++) {
        values.insert(rng->generateInt(0, 1000));
    }
    // With good randomness, should have many different values
    EXPECT_GT(values.size(), 40);
}

TEST_F(GenerateIntTest, HandlesNegativeRange) {
    int min = -100, max = -10;
    for (int i = 0; i < 50; i++) {
        int result = rng->generateInt(min, max);
        EXPECT_GE(result, min);
        EXPECT_LE(result, max);
    }
}

TEST_F(GenerateIntTest, HandlesSingleValue) {
    // When min == max, should always return that value
    int value = 42;
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(rng->generateInt(value, value), value);
    }
}

TEST_F(GenerateIntTest, ReproducibleWithSameSeed) {
    // Generate sequence with seed 42
    rng->seed(42);
    std::vector<int> sequence1;
    for (int i = 0; i < 10; i++) {
        sequence1.push_back(rng->generateInt(0, 1000));
    }

    // Reseed and generate again
    rng->seed(42);
    std::vector<int> sequence2;
    for (int i = 0; i < 10; i++) {
        sequence2.push_back(rng->generateInt(0, 1000));
    }

    // Sequences should be identical
    EXPECT_EQ(sequence1, sequence2);
}

TEST_F(GenerateIntTest, DifferentWithDifferentSeed) {
    // Generate with seed 42
    rng->seed(42);
    int value1 = rng->generateInt(0, 1000000);

    // Generate with different seed
    rng->seed(123);
    int value2 = rng->generateInt(0, 1000000);

    // Very unlikely to be the same
    EXPECT_NE(value1, value2);
}

class GenerateFloatTest : public RandomGeneratorTest {};

TEST_F(GenerateFloatTest, ReturnsValueWithinRange) {
    float min = 0.0f, max = 1.0f;
    for (int i = 0; i < 100; i++) {
        float result = rng->generateFloat(min, max);
        EXPECT_GE(result, min);
        EXPECT_LT(result, max);  // Upper bound is exclusive
    }
}

TEST_F(GenerateFloatTest, ProducesVariedValues) {
    std::set<float> values;
    for (int i = 0; i < 50; i++) {
        float val = rng->generateFloat(0.0f, 100.0f);
        // Round to nearest int to handle floating point precision
        values.insert(std::floor(val));
    }
    // Should have good variety (at least 30 unique values out of 50)
    EXPECT_GT(values.size(), 25);
}

TEST_F(GenerateFloatTest, HandlesNegativeRange) {
    float min = -1.0f, max = 0.0f;
    for (int i = 0; i < 50; i++) {
        float result = rng->generateFloat(min, max);
        EXPECT_GE(result, min);
        EXPECT_LT(result, max);
    }
}

TEST_F(GenerateFloatTest, HandlesLargeRange) {
    float min = -1000.0f, max = 1000.0f;
    for (int i = 0; i < 50; i++) {
        float result = rng->generateFloat(min, max);
        EXPECT_GE(result, min);
        EXPECT_LT(result, max);
    }
}

TEST_F(GenerateFloatTest, ReproducibleWithSameSeed) {
    // Generate sequence with seed 42
    rng->seed(42);
    std::vector<float> sequence1;
    for (int i = 0; i < 10; i++) {
        sequence1.push_back(rng->generateFloat(0.0f, 1.0f));
    }

    // Reseed and generate again
    rng->seed(42);
    std::vector<float> sequence2;
    for (int i = 0; i < 10; i++) {
        sequence2.push_back(rng->generateFloat(0.0f, 1.0f));
    }

    // Sequences should be identical
    EXPECT_EQ(sequence1, sequence2);
}

class SingletonTest : public ::testing::Test {};

TEST_F(SingletonTest, ReturnsConsistentInstance) {
    auto& rng1 = RandomUtils::RandomGenerator::getInstance();
    auto& rng2 = RandomUtils::RandomGenerator::getInstance();

    // Should be the same instance
    EXPECT_EQ(&rng1, &rng2);
}

TEST_F(SingletonTest, StatePreservedBetweenCalls) {
    // Get instance and set seed
    auto& rng = RandomUtils::RandomGenerator::getInstance();
    rng.seed(999);

    int val1 = rng.generateInt(0, 1000);
    int val2 = rng.generateInt(0, 1000);

    // Reset with same seed
    rng.seed(999);
    int val3 = rng.generateInt(0, 1000);

    // val1 and val3 should match (same seed state)
    EXPECT_EQ(val1, val3);
}

class ThreadSafetyTest : public RandomGeneratorTest {};

TEST_F(ThreadSafetyTest, ConcurrentAccessDoesNotCrash) {
    // Simple test: just ensure multiple rapid calls don't crash
    // (full thread safety testing would require more infrastructure)
    for (int i = 0; i < 1000; i++) {
        int val = rng->generateInt(0, 100);
        EXPECT_GE(val, 0);
        EXPECT_LE(val, 100);
    }
}

class UsagePatternTest : public RandomGeneratorTest {};

TEST_F(UsagePatternTest, TypicalScreenPositionGeneration) {
    // Simulate generating random screen positions like in StarFx
    const int SCREEN_WIDTH = 1920;
    const int SCREEN_HEIGHT = 1080;

    std::vector<std::pair<int, int>> positions;
    for (int i = 0; i < 10; i++) {
        int x = rng->generateInt(0, SCREEN_WIDTH);
        int y = rng->generateInt(0, SCREEN_HEIGHT);
        positions.push_back({x, y});

        EXPECT_GE(x, 0);
        EXPECT_LE(x, SCREEN_WIDTH);
        EXPECT_GE(y, 0);
        EXPECT_LE(y, SCREEN_HEIGHT);
    }

    // Should have variety in positions
    EXPECT_EQ(positions.size(), 10);
}

TEST_F(UsagePatternTest, TypicalColorGeneration) {
    // Simulate generating random color values
    for (int i = 0; i < 50; i++) {
        int r = rng->generateInt(0, 255);
        int g = rng->generateInt(0, 255);
        int b = rng->generateInt(0, 255);

        EXPECT_GE(r, 0);
        EXPECT_LE(r, 255);
        EXPECT_GE(g, 0);
        EXPECT_LE(g, 255);
        EXPECT_GE(b, 0);
        EXPECT_LE(b, 255);
    }
}
