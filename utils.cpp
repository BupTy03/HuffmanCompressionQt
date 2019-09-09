#include "utils.hpp"

BitsBuffer read_bits(std::istream& inputStream)
{
    BitsBuffer buffer;
    while(true) {
        std::uint8_t currentByte = 0;
        read(inputStream, currentByte);
        if(inputStream.eof()) {
            break;
        }

        for(int bitIndex = 0; bitIndex < BITS_IN_BYTE; ++bitIndex) {
            buffer.push_back(currentByte & (1 << (BITS_IN_BYTE - bitIndex - 1)));
        }
    }
    return buffer;
}

BytesBuffer read_bytes(std::istream& inputStream)
{
    BytesBuffer buffer;
    while (inputStream) {
        std::uint8_t currByte = 0;
        read(inputStream, currByte);
        buffer.push_back(currByte);
    }
    return buffer;
}

std::uint8_t write_bits(const BitsBuffer& bitsBuffer, std::ostream& outputStream)
{
    std::uint8_t bitIndex = 0;
    std::uint8_t byteOfCode = 0;
    for(bool codeBit : bitsBuffer) {
        byteOfCode |= codeBit << (BITS_IN_BYTE - bitIndex - 1);
        ++bitIndex;

        if(bitIndex >= BITS_IN_BYTE) {
            write(outputStream, byteOfCode);
            byteOfCode = 0;
            bitIndex = 0;
        }
    }

    if(bitIndex > 0) {
        write(outputStream, byteOfCode);
    }

    const auto offset = static_cast<std::uint8_t>(BITS_IN_BYTE - bitIndex);
    return offset;
}

void write_bytes(const BytesBuffer& bytesBuffer, std::ostream& outputStream)
{
    for(const auto currByte : bytesBuffer) {
        write(outputStream, currByte);
    }
}
