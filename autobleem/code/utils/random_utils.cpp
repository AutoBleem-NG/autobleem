//
// Random number generation utilities
// Modern C++11 replacement for util.cpp getRandomNumber/getRandomIndex
// Uses Mersenne Twister (mt19937) instead of C stdlib rand()
//
#include "random_utils.h"
#include <chrono>

namespace RandomUtils {

// Seed with high-resolution clock for better entropy
RandomGenerator::RandomGenerator() {
    auto now = std::chrono::high_resolution_clock::now();
    auto seed = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    generator_.seed(static_cast<uint32_t>(seed));
}

// Thread-safe static initialization (C++11)
RandomGenerator &RandomGenerator::getInstance() {
    static RandomGenerator instance;
    return instance;
}

// Generate random integer in range [min, max]
int RandomGenerator::generateInt(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator_);
}

// Generate random float in range [min, max)
float RandomGenerator::generateFloat(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator_);
}

// Reseed the generator (useful for reproducible tests)
void RandomGenerator::seed(uint32_t seed_value) { generator_.seed(seed_value); }

} // namespace RandomUtils
