#include "utils.hpp"

#include <iterator>
#include <algorithm>

BitsBuffer read_bits(std::istream& inputStream)
{
    BitsBuffer buffer;
    inputStream.unsetf(std::ios::skipws);
    std::for_each(std::istream_iterator<BytesBuffer::value_type>(inputStream), std::istream_iterator<BytesBuffer::value_type>(),
    [&buffer](const std::uint8_t currByte){
        for(int bitIndex = 0; bitIndex < BITS_IN_BYTE; ++bitIndex) {
            buffer.push_back(currByte & (1 << (BITS_IN_BYTE - bitIndex - 1)));
        }
    });
    return buffer;
}

BytesBuffer read_bytes(std::istream& inputStream)
{
    BytesBuffer buffer;
    inputStream.unsetf(std::ios::skipws);
    std::copy(std::istream_iterator<BytesBuffer::value_type>(inputStream),
              std::istream_iterator<BytesBuffer::value_type>(), std::back_inserter(buffer));
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
    outputStream.unsetf(std::ios::skipws);
    std::copy(std::cbegin(bytesBuffer), std::cend(bytesBuffer), std::ostream_iterator<std::uint8_t>(outputStream));
}
