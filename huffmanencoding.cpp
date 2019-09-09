#include "huffmanencoding.hpp"
#include "utils.hpp"
#include "htree.hpp"

#include <fstream>
#include <cassert>

void write_header(const HTree& tree, std::ostream& outputStream)
{
    const auto& dict = tree.huffmanDict();

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
    for(std::size_t byteIndex = 0; byteIndex < dict.size(); ++byteIndex) {
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

    // writing bits
    constexpr int bitsInByte = 8;
    std::uint8_t bitIndex = 0;
    std::uint8_t byteOfCode = 0;
    for(const auto& symbolCode : dict) {
        if(symbolCode.empty()) {
            continue;
        }

        for(bool codeBit : symbolCode) {
            byteOfCode |= codeBit << (bitsInByte - bitIndex - 1);
            ++bitIndex;

            if(bitIndex >= bitsInByte) {
                write(outputStream, byteOfCode);
                byteOfCode = 0;
                bitIndex = 0;
            }
        }
    }

    if(bitIndex > 0) {
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

void compress_data(const HTree& tree, std::istream& inputStream, std::ostream& outputStream)
{
    const auto bitsBuffer = tree.encodeBytes(read_bytes(inputStream));

    const auto pos = outputStream.tellp();
    outputStream.seekp(pos + std::ostream::off_type(1));

    // writing bits
    constexpr int bitsInByte = 8;
    std::uint8_t bitIndex = 0;
    std::uint8_t byteOfCode = 0;
    for(bool codeBit : bitsBuffer) {
        byteOfCode |= codeBit << (bitsInByte - bitIndex - 1);
        ++bitIndex;

        if(bitIndex >= bitsInByte) {
            write(outputStream, byteOfCode);
            byteOfCode = 0;
            bitIndex = 0;
        }
    }

    if(bitIndex > 0) {
        write(outputStream, byteOfCode);
    }

    const auto offset = static_cast<std::uint8_t>(bitsInByte - bitIndex);
    outputStream.clear();
    outputStream.seekp(pos);
    write(outputStream, offset);
}

void read_header(std::istream& inputStream, HTree& tree)
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
    constexpr int bitsInByte = 8;
    BitsBuffer bits;
    while(inputStream && inputStream.tellg() < header.offset) {
        std::uint8_t currentByte = 0;
        read(inputStream, currentByte);
        for(int bitIndex = 0; bitIndex < bitsInByte; ++bitIndex) {
            bits.push_back(currentByte & (1 << (bitsInByte - bitIndex - 1)));
        }
    }

    // fill dict
    HuffmanDict dict;
    auto it = std::cbegin(bits);
    for(const auto& entry : entries) {
        auto& currBitCode = dict.at(entry.symbol);
        std::copy_n(it, entry.count, std::back_inserter(currBitCode));
        it += entry.count;
    }

    tree.setHuffmanDict(dict);
}

void decompress_data(const HTree& tree, std::istream& inputStream, std::ostream& outputStream)
{
    // reading offset
    std::uint8_t offset = 0;
    read(inputStream, offset);

    // reading data
    BitsBuffer bits = read_bits(inputStream);

    // erasing insignificant bits from the end
    assert(std::cend(bits) - offset >= std::cbegin(bits));
    bits.erase(std::cend(bits) - offset, std::cend(bits));

    const auto bytes = tree.decodeBits(bits);
    for(const auto currByte : bytes) {
        write(outputStream, currByte);
    }
}


void compress_file(const std::string& from, const std::string& to)
{
    std::ifstream from_file(from);
    if(!from_file) {
        return;
    }

    HTree tree;
    tree.setStream(from_file);

    std::ofstream to_file(to);
    if(!to_file) {
        return;
    }
    write_header(tree, to_file);
    from_file.clear();
    from_file.seekg(0);

    compress_data(tree, from_file, to_file);
    from_file.close();
    to_file.close();
}

void decompress_file(const std::string& from, const std::string& to)
{
    std::ifstream from_huffman_file(from);

    HTree tree;
    read_header(from_huffman_file, tree);

    std::ofstream to_file(to);
    decompress_data(tree, from_huffman_file, to_file);
    from_huffman_file.close();
    to_file.close();
}
