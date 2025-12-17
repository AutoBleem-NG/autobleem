//
// Binary I/O Reader for parsing binary game files (BIN/PBP)
// Extracted from util.cpp
// Original author: screemer
//
#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace BinaryIO {

// Binary file reader for parsing PlayStation game metadata
class Reader {
  public:
    explicit Reader(std::ifstream *stream);

    unsigned char readByte();
    uint32_t readDword(); // Little-endian
    std::string readFixedString(int size);
    std::string readNullTerminatedString();
    void skipZeros();

  private:
    std::ifstream *stream_;
};

} // namespace BinaryIO
