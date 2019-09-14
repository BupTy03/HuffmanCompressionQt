#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

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

#endif // UTILS_HPP
