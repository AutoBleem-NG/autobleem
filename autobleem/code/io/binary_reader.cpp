// Binary I/O Reader for parsing binary game files (BIN/PBP)
// Original author: screemer
#include "binary_reader.h"

#include <ios>
#include <vector>

namespace BinaryIO {

Reader::Reader(std::ifstream *stream) : stream_(stream) {}

bool Reader::isValid() const { return stream_ != nullptr && stream_->good(); }

unsigned char Reader::readByte() {
    unsigned char c = 0;
    if (stream_ != nullptr) {
        stream_->read(reinterpret_cast<char *>(&c), 1);
    }
    return c;
}

uint32_t Reader::readDword() {
    uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
        uint32_t byte = readByte();
        result |= (byte << (i * 8));
    }
    return result;
}

std::string Reader::readFixedString(size_t size) {
    if (stream_ == nullptr || size == 0) {
        return "";
    }
    std::vector<char> buffer(size + 1, '\0');
    stream_->read(buffer.data(), static_cast<std::streamsize>(size));
    return std::string(buffer.data());
}

std::string Reader::readNullTerminatedString() {
    std::string str;
    if (stream_ == nullptr) {
        return str;
    }
    char c = static_cast<char>(readByte());
    while (!stream_->eof() && !stream_->fail() && c != 0) {
        str += c;
        c = static_cast<char>(readByte());
    }
    return str;
}

void Reader::skipZeros() {
    if (stream_ == nullptr) {
        return;
    }
    char c = static_cast<char>(readByte());
    while (!stream_->eof() && !stream_->fail() && c == 0) {
        c = static_cast<char>(readByte());
    }
    if (!stream_->eof() && !stream_->fail()) {
        stream_->seekg(-1, std::ios::cur);
    }
}

} // namespace BinaryIO
