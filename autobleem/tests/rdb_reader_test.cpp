#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "../code/engine/rdb_reader.h"

namespace {

void appendU64BE(std::vector<unsigned char> &out, uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8)
        out.push_back(static_cast<unsigned char>((value >> shift) & 0xff));
}

void appendString(std::vector<unsigned char> &out, const std::string &value) {
    ASSERT_LT(value.size(), 32u);
    out.push_back(static_cast<unsigned char>(0xa0 | value.size()));
    out.insert(out.end(), value.begin(), value.end());
}

void appendUint16(std::vector<unsigned char> &out, unsigned value) {
    out.push_back(0xcd);
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    out.push_back(static_cast<unsigned char>(value & 0xff));
}

void appendRecord(std::vector<unsigned char> &out) {
    out.push_back(0x87); // fixmap, 7 entries

    appendString(out, "name");
    appendString(out, "Puzzle & Action (USA)");
    appendString(out, "region");
    appendString(out, "USA");
    appendString(out, "serial");
    appendString(out, "SLUS-12345-01");
    appendString(out, "publisher");
    appendString(out, "Example Co., Ltd.");
    appendString(out, "developer");
    appendString(out, "Example Dev");
    appendString(out, "releaseyear");
    appendUint16(out, 1997);
    appendString(out, "users");
    out.push_back(2);
}

std::string testPath(const std::string &name) { return "/tmp/" + name + "_" + std::to_string(getpid()) + ".rdb"; }

bool writeFile(const std::string &path, const std::vector<unsigned char> &data) {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out.is_open())
        return false;
    out.write(reinterpret_cast<const char *>(data.data()), data.size());
    return out.good();
}

std::vector<unsigned char> makeRdb(uint64_t metadataOffset, const std::vector<unsigned char> &records,
                                   const std::vector<unsigned char> &metadata = {}) {
    std::vector<unsigned char> out;
    const unsigned char magic[] = {'R', 'A', 'R', 'C', 'H', 'D', 'B', '\0'};
    out.insert(out.end(), magic, magic + sizeof(magic));
    appendU64BE(out, metadataOffset);
    out.insert(out.end(), records.begin(), records.end());
    out.insert(out.end(), metadata.begin(), metadata.end());
    return out;
}

} // namespace

TEST(RdbReaderTest, ReadsRecordsWithZeroMetadataOffset) {
    std::vector<unsigned char> records;
    appendRecord(records);
    records.push_back(0xc0); // nil sentinel

    const std::string path = testPath("zero_offset");
    ASSERT_TRUE(writeFile(path, makeRdb(0, records)));

    RdbReader reader;
    EXPECT_TRUE(reader.open(path));
    EXPECT_TRUE(reader.isValid());
    EXPECT_EQ(1u, reader.size());

    const auto *byName = reader.findByName("Puzzle & Action (USA)");
    ASSERT_NE(nullptr, byName);
    EXPECT_EQ("Example Co., Ltd.", byName->publisher);
    EXPECT_EQ(1997, byName->releaseyear);
    EXPECT_EQ(2, byName->users);

    const auto *byPrefixSerial = reader.findBySerial("SLUS-12345");
    ASSERT_NE(nullptr, byPrefixSerial);
    EXPECT_EQ("Puzzle & Action (USA)", byPrefixSerial->name);

    std::remove(path.c_str());
}

TEST(RdbReaderTest, StopsAtMetadataOffset) {
    std::vector<unsigned char> records;
    appendRecord(records);
    records.push_back(0xc0); // nil sentinel
    const uint64_t metadataOffset = 16 + records.size();

    const std::string path = testPath("with_metadata");
    ASSERT_TRUE(writeFile(path, makeRdb(metadataOffset, records, {0xff, 0xff, 0xff})));

    RdbReader reader;
    EXPECT_TRUE(reader.open(path));
    EXPECT_EQ(1u, reader.size());

    std::remove(path.c_str());
}

TEST(RdbReaderTest, RejectsInvalidMagic) {
    const std::string path = testPath("bad_magic");
    ASSERT_TRUE(writeFile(path, {'B', 'A', 'D'}));

    RdbReader reader;
    EXPECT_FALSE(reader.open(path));
    EXPECT_FALSE(reader.isValid());

    std::remove(path.c_str());
}

TEST(RdbReaderTest, LoadsRealDatabaseWhenProvided) {
    const char *path = std::getenv("AUTOBLEEM_TEST_RDB");
    if (path == nullptr || path[0] == '\0') {
        GTEST_SKIP() << "AUTOBLEEM_TEST_RDB not set";
    }

    RdbReader reader;
    ASSERT_TRUE(reader.open(path));
    EXPECT_TRUE(reader.isValid());
    EXPECT_GT(reader.size(), 1000u);
}
