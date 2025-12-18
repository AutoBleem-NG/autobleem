// Binary I/O Reader for parsing binary game files (BIN/PBP)
// Original author: screemer
#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace BinaryIO {

// Binary file reader for parsing PlayStation game metadata.
// Non-owning wrapper around std::ifstream - caller manages stream lifetime.
class Reader {
  public:
    explicit Reader(std::ifstream *stream);

    // Returns true if stream is valid and ready for reading
    bool isValid() const;

    unsigned char readByte();
    uint32_t readDword(); // Little-endian 32-bit
    std::string readFixedString(size_t size);
    std::string readNullTerminatedString();

    // Skip consecutive zero bytes, positioning at first non-zero byte
    void skipZeros();

  private:
    std::ifstream *stream_;
};

} // namespace BinaryIO
