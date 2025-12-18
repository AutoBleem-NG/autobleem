// Unit tests for DebugTimer
#include <gtest/gtest.h>

#include "../code/debug_timer.h"

#include <thread>
#include <chrono>

class DebugTimerTest : public ::testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test that DebugTimer can be constructed with a description
TEST_F(DebugTimerTest, ConstructsWithDescription) {
    // Should not throw or crash
    DebugTimer timer("test timer");
}

// Test that DebugTimer can be constructed with empty description
TEST_F(DebugTimerTest, ConstructsWithEmptyDescription) {
    DebugTimer timer("");
}

// Test that DebugTimer destructor runs without issue
TEST_F(DebugTimerTest, DestructorRunsCleanly) {
    {
        DebugTimer timer("scoped timer");
        // Timer goes out of scope here
    }
    // If we reach here, destructor ran successfully
    SUCCEED();
}

// Test RAII behavior - timer starts and stops automatically with scope
TEST_F(DebugTimerTest, RAIIBehavior) {
    {
        DebugTimer timer("raii test");
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    SUCCEED();
}

// Test multiple timers in sequence
TEST_F(DebugTimerTest, MultipleTimersInSequence) {
    {
        DebugTimer timer1("first");
    }
    {
        DebugTimer timer2("second");
    }
    {
        DebugTimer timer3("third");
    }
    SUCCEED();
}

// Test nested timers
TEST_F(DebugTimerTest, NestedTimers) {
    DebugTimer outer("outer");
    {
        DebugTimer inner("inner");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    SUCCEED();
}

#ifndef NDEBUG
// Debug-only tests that check internal state
TEST_F(DebugTimerTest, TicksStartIsInitialized) {
    DebugTimer timer("ticks test");
    // In debug builds, ticks_start should be non-zero after construction
    EXPECT_GT(timer.ticks_start, 0u);
}

TEST_F(DebugTimerTest, DescriptionIsStored) {
    DebugTimer timer("my description");
    EXPECT_EQ(timer.description, "my description");
}

TEST_F(DebugTimerTest, TicksEndIsZeroBeforeDestruction) {
    DebugTimer timer("end ticks test");
    EXPECT_EQ(timer.ticks_end, 0u);
}
#endif
