#include "engine/disc_suffix.h"

#include <gtest/gtest.h>

namespace {

void expectMatch(const std::string &input, const std::string &expectedBase, int expectedDisc) {
    const DiscSuffix r = parseDiscSuffix(input);
    EXPECT_TRUE(r.matched()) << "expected match for: " << input;
    EXPECT_EQ(expectedBase, r.base) << "base mismatch for: " << input;
    EXPECT_EQ(expectedDisc, r.disc) << "disc mismatch for: " << input;
}

void expectNoMatch(const std::string &input) {
    const DiscSuffix r = parseDiscSuffix(input);
    EXPECT_FALSE(r.matched()) << "expected no match for: " << input;
    EXPECT_EQ(input, r.base);
    EXPECT_EQ(0, r.disc);
}

} // namespace

TEST(DiscSuffixTest, ParenDisc) {
    expectMatch("Final Fantasy VII (USA) (Disc 1)", "Final Fantasy VII (USA)", 1);
    expectMatch("Metal Gear Solid (USA) (Disc 2)", "Metal Gear Solid (USA)", 2);
    expectMatch("Chrono Cross (Disc 1)", "Chrono Cross", 1);
}

TEST(DiscSuffixTest, ParenDisk) {
    expectMatch("Some Game (Disk 1)", "Some Game", 1);
    expectMatch("Other (Disk 12)", "Other", 12);
}

TEST(DiscSuffixTest, ParenCD) {
    expectMatch("Lunar SSSC (CD 1)", "Lunar SSSC", 1);
    expectMatch("Lunar SSSC (CD1)", "Lunar SSSC", 1);
    expectMatch("Lunar SSSC (CD 2)", "Lunar SSSC", 2);
}

TEST(DiscSuffixTest, DashDisc) {
    expectMatch("Foo - Disc 1", "Foo", 1);
    expectMatch("Foo - Disk 2", "Foo", 2);
    expectMatch("Foo - CD 3", "Foo", 3);
}

TEST(DiscSuffixTest, CaseInsensitive) {
    expectMatch("Game (DISC 1)", "Game", 1);
    expectMatch("Game (disc 2)", "Game", 2);
    expectMatch("Game - DISK 5", "Game", 5);
}

TEST(DiscSuffixTest, TrailingWhitespace) {
    expectMatch("Game (Disc 1)   ", "Game", 1);
    expectMatch("Game (Disc 1  )", "Game", 1);
    expectMatch("Game ( Disc 1 )", "Game", 1);
}

TEST(DiscSuffixTest, NoMatch) {
    expectNoMatch("");
    expectNoMatch("Game");
    expectNoMatch("Game (USA)");
    expectNoMatch("Game (Rev 1)");
    expectNoMatch("Game (Disc A)");
    expectNoMatch("Game (Disc)");
    expectNoMatch("Game (Disc 1) extra");
    expectNoMatch("Game - Final");
}

TEST(DiscSuffixTest, DiscMarkerMustBeLastParen) {
    // (Disc 1) is not the final paren — there's (Rev 1) after it.
    expectNoMatch("Game (Disc 1) (Rev 1)");
}

TEST(DiscSuffixTest, MalformedParens) {
    expectNoMatch("Game )Disc 1(");
    expectNoMatch("Game Disc 1)");
}
