#include "mainwindow.hpp"
#include "htree.hpp"

#include <iostream>
#include <fstream>
#include <cassert>

#include <QApplication>

struct HuffmanHeader
{
    std::uint8_t header[4]{'\0'}; // заголовок "HAFF"
    std::uint16_t count = 0;      // кол-во записей SymbolEntry
    std::uint16_t offset = 0;     // оффсет до data
};

struct SymbolEntry
{
    std::uint8_t symbol = 0;    // символ (байт)
    std::uint8_t count = 0;     // кол-во бит кода символа
};


void print(const BitsBuffer& bits) { std::copy(std::cbegin(bits), std::cend(bits), std::ostream_iterator<bool>(std::cout)); }
void print(const BytesBuffer& bytes) { std::copy(std::cbegin(bytes), std::cend(bytes), std::ostream_iterator<std::uint8_t>(std::cout)); }
void error(const char* message) { std::cerr << "Error: " << message << std::endl; }

BitsBuffer encode(const HuffmanDict& dict, const BytesBuffer& message)
{
    BitsBuffer result;
    for(auto ch : message) {
        const auto& bits = dict.at(static_cast<std::size_t>(ch));
        std::copy(std::cbegin(bits), std::cend(bits), std::back_inserter(result));
    }
    return result;
}

BitsBuffer encode(const HuffmanDict& dict, std::istream& stream)
{
    BitsBuffer result;
    while (stream) {
        std::uint8_t ch = 0;
        stream.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        const auto& bits = dict.at(static_cast<std::size_t>(ch));
        std::copy(std::cbegin(bits), std::cend(bits), std::back_inserter(result));
    }
    return result;
}

BytesBuffer decode(const HuffmanDict& dict, const BitsBuffer& bitsBuffer)
{
    BytesBuffer result;
    for(std::size_t bitIndex = 0; bitIndex < bitsBuffer.size(); ) {
        bool bitSequenceFound = false;
        for(std::size_t byteIndex = 0; byteIndex < dict.size(); ++byteIndex) {
            auto& codeBits = dict.at(byteIndex);
            if(codeBits.empty()) continue;

            auto byteBitsIt = std::mismatch(std::cbegin(codeBits), std::cend(codeBits), std::cbegin(bitsBuffer) + static_cast<int>(bitIndex));
            if(byteBitsIt.first == std::cend(codeBits)) {
                result.push_back(static_cast<std::uint8_t>(byteIndex));
                bitIndex = static_cast<std::size_t>(std::distance(std::cbegin(bitsBuffer), byteBitsIt.second));
                bitSequenceFound = true;
                break;
            }
        }

        if(!bitSequenceFound) {
            error("Bits sequence is not found!");
            break;
        }
    }
    return result;
}

template<class T>
void write(std::ostream& outputStream, const T& data) { outputStream.write(reinterpret_cast<const char*>(&data), sizeof(T)); }

template<class T>
void read(std::istream& inputStream, T& val) { inputStream.read(reinterpret_cast<char*>(&val), sizeof(T)); }

std::uint8_t setbit(const std::uint8_t value, const std::uint8_t position) { return (value | (1 << position)); }
bool getbit(const std::uint8_t value, const std::uint8_t position) { return (value & (1 << position)) != 0; }

void write_header(const HuffmanDict& dict, std::ostream& outputStream)
{
    // writing header
    constexpr std::uint8_t huffHeader[] = {'H', 'A', 'F', 'F'};
    std::copy(std::cbegin(huffHeader), std::cend(huffHeader), std::ostream_iterator<std::uint8_t>(outputStream));

    const auto hasCode = [](const BitsBuffer& bitCode){
        return !bitCode.empty();
    };

    // writing count entries
    const auto countEntries = static_cast<std::uint16_t>(std::count_if(std::cbegin(dict), std::cend(dict), hasCode));
    write(outputStream, countEntries);

    const auto posOfOffset = outputStream.tellp();
    outputStream.seekp(posOfOffset + std::ostream::off_type(2));

    // writing entries
    for(std::size_t byteIndex = 0; byteIndex < dict.size(); ++byteIndex)
    {
        const auto& codeBits = dict.at(byteIndex);
        assert(codeBits.size() <= std::numeric_limits<std::uint8_t>::max());

        if(codeBits.empty()) {
            continue;
        }

        SymbolEntry entry{
            static_cast<std::uint8_t>(byteIndex),      // symbol
            static_cast<std::uint8_t>(codeBits.size()) // count bits
        };

        write(outputStream, entry);
    }

    qDebug() << outputStream.tellp();

//    BitsBuffer bits;
//    for(const auto& symbolCode : dict)
//    {
//        if(symbolCode.empty()) {
//            continue;
//        }

//        for(bool codeBit : symbolCode)
//        {
//            bits.push_back(codeBit);
//        }
//    }

//    print(bits);
//    std::cout << std::endl;

//    int bitIndex = 0;
//    std::uint8_t byteOfCode = 0;
//    for(bool bit : bits) {
//        byteOfCode |= (bit << (8 - bitIndex - 1));
//        ++bitIndex;
//        if(bitIndex >= 8) {
//            write(outputStream, byteOfCode);
//            byteOfCode = 0;
//            bitIndex = 0;
//        }
//    }



    // writing bits
    constexpr int bitsInByte = 8;
    int bitIndex = 0;
    std::uint8_t byteOfCode = 0;
    for(const auto& symbolCode : dict)
    {
        if(symbolCode.empty()) {
            continue;
        }

        for(bool codeBit : symbolCode)
        {
            byteOfCode |= codeBit << (bitsInByte - bitIndex - 1);
            ++bitIndex;

            if(bitIndex >= bitsInByte) {
                write(outputStream, byteOfCode);
                byteOfCode = 0;
                bitIndex = 0;
            }
        }
    }

    if(bitIndex < 8) {
        write(outputStream, byteOfCode);
    }

    const auto dataOffset = outputStream.tellp();
    outputStream.clear();
    outputStream.seekp(posOfOffset);
    const auto offset = static_cast<std::uint16_t>(dataOffset);
    write(outputStream, offset);

    outputStream.clear();
    outputStream.seekp(dataOffset);
}

void read_header(std::istream& inputStream, HuffmanDict& dict)
{
    // reading header
    HuffmanHeader header;
    read(inputStream, header);

    // reading entries
    std::vector<SymbolEntry> entries;
    for(std::size_t entryIndex = 0; entryIndex < header.count; ++entryIndex) {
        entries.emplace_back();
        read(inputStream, entries.back());
    }

    // reading bits
    qDebug() << inputStream.tellg();
    constexpr int bitsInByte = 8;
    BitsBuffer bits;
    while(inputStream /*&& inputStream.tellg() < header.offset*/) {
        std::uint8_t currentByte = 0;
        read(inputStream, currentByte);
        for(int bitIndex = 0; bitIndex < bitsInByte; ++bitIndex) {
            bits.push_back(getbit(currentByte, bitsInByte - bitIndex - 1) /*currentByte & (1 << bitIndex)*/);
        }
    }

    auto it = std::cbegin(bits);
    for(const auto& entry : entries) {
        auto& currBitCode = dict.at(entry.symbol);
        std::copy_n(it, entry.count, std::back_inserter(currBitCode));
        it += entry.count;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    std::ifstream ifs("D:\\USER\\Documents\\text.txt");
    if(!ifs) {
        error("Unable to open file!");
        return 0;
    }

    HTree tree;
    tree.setStream(ifs);
    ifs.close();

    const auto huffmanCodes = tree.huffmanDict();

    // print huffman codes
    for(std::size_t signByte = 0; signByte < huffmanCodes.size(); ++signByte) {
        const auto& bitCode = huffmanCodes.at(signByte);
        if(bitCode.empty()) continue;

        std::cout << "'" << static_cast<char>(signByte) << "' = ";
        print(bitCode);
        std::cout << std::endl;
    }

    std::ofstream ofs("D:\\USER\\Documents\\test_huffman.txt");
    write_header(huffmanCodes, ofs);
    ofs.close();

    HuffmanDict newHuffmanCodes;
    std::ifstream newifs("D:\\USER\\Documents\\test_huffman.txt");
    read_header(newifs, newHuffmanCodes);
    newifs.close();

    // print huffman codes
    for(std::size_t signByte = 0; signByte < newHuffmanCodes.size(); ++signByte) {
        const auto& bitCode = newHuffmanCodes.at(signByte);
        if(bitCode.empty()) continue;

        std::cout << "'" << static_cast<char>(signByte) << "' = ";
        print(bitCode);
        std::cout << std::endl;
    }

/*
    HTree newTree;
    newTree.setHuffmanDict(huffmanCodes);

    // encoding
    ifs.clear();
    ifs.seekg(0, ifs.beg);
    const auto bits = encode(huffmanCodes, ifs);
    ifs.close();
    std::cout << "Encoded(size: " << bits.size() / 8 << " bytes): ";
    print(bits);
    std::cout << std::endl;

    // decoding
    const auto bytes = newTree.decodeBits(bits);
    std::cout << "Decoded(size: " << bytes.size() << " bytes): ";
    print(bytes);
    std::cout << std::endl;
*/
    return a.exec();
}
