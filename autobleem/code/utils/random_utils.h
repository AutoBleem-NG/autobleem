//
// Random number generation utilities
// Modern C++11 replacement for util.cpp getRandomNumber/getRandomIndex
// Uses Mersenne Twister (mt19937) instead of C stdlib rand()
//
#pragma once

#include <random>
#include <cstdint>

namespace RandomUtils {

// Global Mersenne Twister random number generator
// Thread-safe initialization, seeded from system entropy
class RandomGenerator {
  public:
    // Get the singleton instance
    static RandomGenerator &getInstance();

    // Generate random integer in range [min, max]
    int generateInt(int min, int max);

    // Generate random float in range [min, max)
    float generateFloat(float min, float max);

    // Reseed the generator (useful for reproducible tests)
    void seed(uint32_t seed_value);

    // Delete copy/move constructors for singleton
    RandomGenerator(const RandomGenerator &) = delete;
    RandomGenerator &operator=(const RandomGenerator &) = delete;
    RandomGenerator(RandomGenerator &&) = delete;
    RandomGenerator &operator=(RandomGenerator &&) = delete;

  private:
    RandomGenerator();
    std::mt19937 generator_;
};

} // namespace RandomUtils
