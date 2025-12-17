//
// Binary I/O Reader for parsing binary game files (BIN/PBP)
// Extracted from util.cpp
// Original author: screemer
//
#include "binary_reader.h"

#include <ios>

namespace BinaryIO {

Reader::Reader(std::ifstream *stream) : stream_(stream) {}

unsigned char Reader::readByte() {
    unsigned char c;
    stream_->read(reinterpret_cast<char *>(&c), 1);
    return c;
}

// Read 32-bit little-endian integer
uint32_t Reader::readDword() {
    uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
        uint32_t byte = readByte();
        result |= (byte << (i * 8));
    }
    return result;
}

std::string Reader::readFixedString(int size) {
    char str[size + 1];
    str[size] = 0;
    stream_->read(str, size);
    return str;
}

std::string Reader::readNullTerminatedString() {
    std::string str;
    char c = readByte();
    while (!stream_->eof() && !stream_->fail() && c != 0) {
        str += c;
        c = readByte();
    }
    return str;
}

void Reader::skipZeros() {
    char c = readByte();
    while (!stream_->eof() && !stream_->fail() && c == 0) {
        c = readByte();
    }
    stream_->seekg(-1, std::ios::cur);
}

} // namespace BinaryIO
