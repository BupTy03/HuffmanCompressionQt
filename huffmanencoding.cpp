#include "huffmanencoding.hpp"
#include "bits_utils.hpp"
#include "utils.hpp"
#include "htree.hpp"
#include "istreambitsiterator.hpp"
#include "ostreambitsiterator.hpp"

#include <fstream>
#include <cassert>
#include <iterator>


void write_header(const HTree& tree, std::ostream& outputStream)
{
    const auto& dict = tree.huffmanDict();

    // writing header
    constexpr std::array<std::uint8_t, 4> huffHeader = {'H', 'A', 'F', 'F'};
    std::copy(std::cbegin(huffHeader), std::cend(huffHeader), std::ostreambuf_iterator<char>(outputStream));

    const auto hasCode = [](const BitsBuffer& bitCode){
        return !bitCode.empty();
    };

    // writing count entries
    const auto countEntries = static_cast<std::uint16_t>(std::count_if(std::cbegin(dict), std::cend(dict), hasCode));
    write(outputStream, countEntries);

    const auto posOfOffset = outputStream.tellp();
    outputStream.seekp(posOfOffset + std::ostream::off_type(2));

    // writing entries
    std::vector<SymbolEntry> entries;
    entries.reserve(countEntries);
    for(std::size_t byteIndex = 0; byteIndex < dict.size(); ++byteIndex) {
        const auto& codeBits = dict.at(byteIndex);
        assert(codeBits.size() <= std::numeric_limits<std::uint8_t>::max());

        if(codeBits.empty()) {
            continue;
        }

        entries.push_back(SymbolEntry{static_cast<std::uint8_t>(byteIndex), static_cast<std::uint8_t>(codeBits.size())});
    }

    outputStream.write(reinterpret_cast<const char*>(entries.data()), std::streamsize(sizeof(SymbolEntry) * entries.size()));

    // writing bits
    OstreamBitsIterator outIt(outputStream);
    for(const auto& bitCode : dict) {
        std::copy(std::cbegin(bitCode), std::cend(bitCode), outIt);
    }

    if(outIt.currentBit() > 0) {
        outIt.flush();
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
    inputStream.unsetf(std::ios::skipws);
    outputStream.unsetf(std::ios::skipws);

    const auto pos = outputStream.tellp();
    outputStream.clear();
    outputStream.seekp(pos + std::ostream::off_type(1));

    OstreamBitsIterator outIt(outputStream);
    const auto offset = tree.encodeBytes(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>(), outIt);
    if(offset > 0) {
        outIt.flush();
    }

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
    std::vector<SymbolEntry> entries(header.count);
    inputStream.read(reinterpret_cast<char*>(entries.data()), std::streamsize(sizeof(SymbolEntry) * entries.size()));

    // reading bits
    std::vector<bool> bits;
    while(inputStream && inputStream.tellg() < header.offset) {
        std::uint8_t currentByte = 0;
        read(inputStream, currentByte);
        for(std::size_t bitIndex = 0; bitIndex < BITS_IN_BYTE; ++bitIndex) {
            bits.push_back(get_bit(currentByte, bitIndex));
        }
    }

    // fill dict
    HuffmanDict dict(COUNT_FREQUENCIES);
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

    // decoding data
    outputStream.unsetf(std::ios::skipws);
    tree.decodeBits(IstreamBitsIterator(inputStream), IstreamBitsIterator(), std::ostreambuf_iterator<char>(outputStream), offset);
}


void compress_file(const std::string& from, const std::string& to)
{
    std::ifstream from_file(from, std::ios::in | std::ios::binary);
    from_file.unsetf(std::ios::skipws);
    if(!from_file) {
        throw std::runtime_error{"Unable to open file: \"" + from + "\" to read"};
    }

    HTree tree;
    tree.setData(std::istreambuf_iterator<char>(from_file), std::istreambuf_iterator<char>());

    std::ofstream to_file(to, std::ios::out | std::ios::binary);
    to_file.unsetf(std::ios::skipws);
    if(!to_file) {
        throw std::runtime_error{"Unable to open file: \"" + to + "\" to write"};
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
    std::ifstream from_huffman_file(from, std::ios::in | std::ios::binary);
    from_huffman_file.unsetf(std::ios::skipws);
    if(!from_huffman_file) {
        throw std::runtime_error{"Unable to open file: \"" + from + "\" to read"};
    }

    HTree tree;
    read_header(from_huffman_file, tree);

    std::ofstream to_file(to, std::ios::out | std::ios::binary);
    to_file.unsetf(std::ios::skipws);
    if(!to_file) {
        throw std::runtime_error{"Unable to open file: \"" + to + "\" to write"};
    }
    decompress_data(tree, from_huffman_file, to_file);
    from_huffman_file.close();
    to_file.close();
}
