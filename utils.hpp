#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>

using BytesBuffer = std::vector<std::uint8_t>;
using BitsBuffer = std::vector<bool>;

constexpr int BITS_IN_BYTE = 8;

template<class T>
void write(std::ostream& outputStream, const T& data)
{
    outputStream.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

template<class T>
void read(std::istream& inputStream, T& val)
{
    inputStream.read(reinterpret_cast<char*>(&val), sizeof(T));
}

BitsBuffer read_bits(std::istream& inputStream);
BytesBuffer read_bytes(std::istream& inputStream);

std::uint8_t write_bits(const BitsBuffer& bitsBuffer, std::ostream& outputStream);
void write_bytes(const BytesBuffer& bytesBuffer, std::ostream& outputStream);

#endif // UTILS_HPP
