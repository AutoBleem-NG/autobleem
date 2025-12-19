// Unit tests for SerialScanner class
//
// Tests serial number parsing, normalization, and region detection.
// The main entry points for these tests are static methods that don't
// require disc images.

#include "engine/serial_scanner.h"
#include <gtest/gtest.h>
#include <string>

//=============================================================================
// Tests for fixSerial()
//=============================================================================

class FixSerialTest : public ::testing::Test {};

TEST_F(FixSerialTest, StandardUSSerial) {
    EXPECT_EQ(SerialScanner::fixSerial("SLUS-00123"), "SLUS-00123");
    EXPECT_EQ(SerialScanner::fixSerial("SCUS-94900"), "SCUS-94900");
}

TEST_F(FixSerialTest, StandardJapanSerial) {
    EXPECT_EQ(SerialScanner::fixSerial("SLPS-00001"), "SLPS-00001");
    EXPECT_EQ(SerialScanner::fixSerial("SLPM-86000"), "SLPM-86000");
    EXPECT_EQ(SerialScanner::fixSerial("SCPS-10000"), "SCPS-10000");
}

TEST_F(FixSerialTest, StandardEuropeSerial) {
    EXPECT_EQ(SerialScanner::fixSerial("SLES-00001"), "SLES-00001");
    EXPECT_EQ(SerialScanner::fixSerial("SCES-00100"), "SCES-00100");
}

TEST_F(FixSerialTest, ReplacesUnderscoreWithDash) {
    EXPECT_EQ(SerialScanner::fixSerial("SLUS_00123"), "SLUS-00123");
    EXPECT_EQ(SerialScanner::fixSerial("SLES_12345"), "SLES-12345");
}

TEST_F(FixSerialTest, RemovesDots) {
    EXPECT_EQ(SerialScanner::fixSerial("SLUS.00123"), "SLUS-00123");
    EXPECT_EQ(SerialScanner::fixSerial("SLUS_012.34"), "SLUS-01234");
}

TEST_F(FixSerialTest, RemovesExistingDashesAndReformats) {
    // Serial with existing dash should still be reformatted correctly
    EXPECT_EQ(SerialScanner::fixSerial("SLUS--00123"), "SLUS-00123");
}

TEST_F(FixSerialTest, HandlesThreeCharPrefix) {
    // LPS, LSP prefixes use 3-character codes
    EXPECT_EQ(SerialScanner::fixSerial("LPS00001"), "LPS-00001");
    EXPECT_EQ(SerialScanner::fixSerial("LSP12345"), "LSP-12345");
}

TEST_F(FixSerialTest, HandlesFourCharPrefix) {
    // Most prefixes use 4-character codes
    EXPECT_EQ(SerialScanner::fixSerial("SLUS00123"), "SLUS-00123");
    EXPECT_EQ(SerialScanner::fixSerial("SLES12345"), "SLES-12345");
}

TEST_F(FixSerialTest, HandlesSerialWithExtraChars) {
    // ISO 9660 version suffix ";1" - the semicolon is skipped but digits are included
    // (Version suffix removal is handled by Isodir::removeVersion, not fixSerial)
    EXPECT_EQ(SerialScanner::fixSerial("SLUS_012.34;1"), "SLUS-012341");
}

TEST_F(FixSerialTest, HandlesNoDash) {
    EXPECT_EQ(SerialScanner::fixSerial("SLUS00123"), "SLUS-00123");
}

TEST_F(FixSerialTest, HandlesEmptyString) {
    EXPECT_EQ(SerialScanner::fixSerial(""), "-");
}

TEST_F(FixSerialTest, HandlesOnlyLetters) {
    EXPECT_EQ(SerialScanner::fixSerial("SLUS"), "SLUS-");
}

TEST_F(FixSerialTest, HandlesOnlyDigits) {
    EXPECT_EQ(SerialScanner::fixSerial("12345"), "-12345");
}

TEST_F(FixSerialTest, HandlesLowerCase) {
    // The function doesn't convert case - it preserves input case
    EXPECT_EQ(SerialScanner::fixSerial("slus_00123"), "slus-00123");
}

TEST_F(FixSerialTest, IgnoresNonDigitsAfterDigitsStart) {
    // Once digits are being processed, non-digits are skipped
    EXPECT_EQ(SerialScanner::fixSerial("SLUS-001X23"), "SLUS-00123");
}

//=============================================================================
// Tests for serialToRegion()
//=============================================================================

class SerialToRegionTest : public ::testing::Test {};

TEST_F(SerialToRegionTest, USSerials) {
    EXPECT_EQ(SerialScanner::serialToRegion("SLUS-00123"), "US");
    EXPECT_EQ(SerialScanner::serialToRegion("SCUS-94900"), "US");
}

TEST_F(SerialToRegionTest, JapanSerials) {
    EXPECT_EQ(SerialScanner::serialToRegion("SLPS-00001"), "Japan");
    EXPECT_EQ(SerialScanner::serialToRegion("SLPM-86000"), "Japan");
    EXPECT_EQ(SerialScanner::serialToRegion("SCPS-10000"), "Japan");
}

TEST_F(SerialToRegionTest, EuropeSerials) {
    EXPECT_EQ(SerialScanner::serialToRegion("SLES-00001"), "Europe-Aus");
    EXPECT_EQ(SerialScanner::serialToRegion("SCES-00100"), "Europe-Aus");
}

TEST_F(SerialToRegionTest, ThirdCharacterDeterminesRegion) {
    // U = US, E = Europe, P = Japan (based on third character)
    EXPECT_EQ(SerialScanner::serialToRegion("XXU"), "US");
    EXPECT_EQ(SerialScanner::serialToRegion("XXE"), "Europe-Aus");
    EXPECT_EQ(SerialScanner::serialToRegion("XXP"), "Japan");
}

TEST_F(SerialToRegionTest, UnknownRegionReturnsEmpty) {
    EXPECT_EQ(SerialScanner::serialToRegion("XXX-12345"), "");
    EXPECT_EQ(SerialScanner::serialToRegion("AAA-12345"), "");
}

TEST_F(SerialToRegionTest, ShortStringReturnsEmpty) {
    EXPECT_EQ(SerialScanner::serialToRegion("SL"), "");
    EXPECT_EQ(SerialScanner::serialToRegion("S"), "");
    EXPECT_EQ(SerialScanner::serialToRegion(""), "");
}

TEST_F(SerialToRegionTest, CaseSensitive) {
    // The function is case-sensitive - lowercase won't match
    EXPECT_EQ(SerialScanner::serialToRegion("slu-00123"), "");
    EXPECT_EQ(SerialScanner::serialToRegion("SLu-00123"), "");
}

//=============================================================================
// Edge Cases
//=============================================================================

class SerialScannerEdgeCasesTest : public ::testing::Test {};

TEST_F(SerialScannerEdgeCasesTest, RoundTripFixSerialAndRegion) {
    // Verify that fixed serials can be used for region detection
    std::string fixed = SerialScanner::fixSerial("SLUS_012.34");
    EXPECT_EQ(SerialScanner::serialToRegion(fixed), "US");

    fixed = SerialScanner::fixSerial("SLES_001_23");
    EXPECT_EQ(SerialScanner::serialToRegion(fixed), "Europe-Aus");

    fixed = SerialScanner::fixSerial("SLPS_000_01");
    EXPECT_EQ(SerialScanner::serialToRegion(fixed), "Japan");
}
