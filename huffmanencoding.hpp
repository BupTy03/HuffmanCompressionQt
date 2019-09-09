#ifndef HUFFMANENCODING_HPP
#define HUFFMANENCODING_HPP

#include <iostream>

struct HuffmanHeader {
    std::uint8_t header[4]{'\0'}; // заголовок "HAFF"
    std::uint16_t count = 0;      // кол-во записей SymbolEntry
    std::uint16_t offset = 0;     // оффсет до data
};

struct SymbolEntry {
    std::uint8_t symbol = 0;    // символ (байт)
    std::uint8_t count = 0;     // кол-во бит кода символа
};

// Data
// std::uint8_t offset; // (кол-во незначащих бит с конца данных)
// BitsBuffer           // биты данных

class HTree;

void write_header(const HTree& tree, std::ostream& outputStream);
void compress_data(const HTree& tree, std::istream& inputStream, std::ostream& outputStream);

void read_header(std::istream& inputStream, HTree& tree);
void decompress_data(const HTree& tree, std::istream& inputStream, std::ostream& outputStream);

void compress_file(const std::string& from, const std::string& to);
void decompress_file(const std::string& from, const std::string& to);

#endif // HUFFMANENCODING_HPP
