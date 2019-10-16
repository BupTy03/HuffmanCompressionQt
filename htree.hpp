#ifndef HTREE_HPP
#define HTREE_HPP

#include "bits_array.hpp"
#include "utils.hpp"

#include <vector>
#include <array>
#include <algorithm>
#include <iostream>


using BytesBuffer = std::vector<std::uint8_t>;
using BitsBuffer = bits_array<std::uint32_t>;
using HuffmanDict = std::vector<BitsBuffer>;
using CharFrequencies = std::array<std::size_t, COUNT_FREQUENCIES>;

struct HTreeNode {
    std::size_t weight = 0;
    std::uint8_t sign = 0;
    int leftNodeID = -1;
    int rightNodeID = -1;
    int parentNodeID = -1;
};

class HTree {
public: // types
    using Node = HTreeNode;
    using Nodes = std::vector<Node>;
    using NodeIDs = std::vector<int>;

public:
    explicit HTree() : huffmanDict_{COUNT_FREQUENCIES} {}
    HuffmanDict huffmanDict() const { return huffmanDict_; }

    template<class It>
    void setData(It first, It last)
    {
        // calculating frequencies
        CharFrequencies frequencies{0};
        std::for_each(first, last, [&frequencies](const std::uint8_t currByte){
            ++frequencies.at(currByte);
        });

        // building tree
        const auto leafs = fillNodes(frequencies);
        buildTree(leafs);
        buildHuffmanDictFromTree(huffmanDict_, leafs);
    }

    void setHuffmanDict(const HuffmanDict& dict);
    void setHuffmanDict(HuffmanDict&& dict);

    template<class ByteIt, class BitIt>
    std::uint8_t encodeBytes(ByteIt first, ByteIt last, BitIt outFirst) const
    {
        for(; first != last; ++first) {
            const auto& currBits = huffmanDict_.at(*first);
            std::copy(std::cbegin(currBits), std::cend(currBits), outFirst);
        }

        return BITS_IN_BYTE - outFirst.currentBit();
    }

    template<class BitIt, class ByteIt>
    void decodeBits(BitIt first, BitIt last, ByteIt outFirst, std::uint8_t bitsOffset) const
    {
        for(; first != last; ++outFirst) {
            int currNodeID = rootID_;
            while(getNode(currNodeID).leftNodeID >= 0 && getNode(currNodeID).rightNodeID >= 0 && first != last) {
                if(first.isLastByte() && first.currentBit() > (BITS_IN_BYTE - bitsOffset - 1)) {
                    return;
                }
                currNodeID = *first ? getNode(currNodeID).rightNodeID : getNode(currNodeID).leftNodeID;
                ++first;
            }
            *outFirst = getNode(currNodeID).sign;
        }
    }

private:
    int makeNode();
    int makeNode(int parentID);
    int makeLeftNode(int parentID);
    int makeRightNode(int parentID);

    Node& getNode(int nodeID) { return nodes_.at(static_cast<std::size_t>(nodeID)); }
    const Node& getNode(int nodeID) const { return nodes_.at(static_cast<std::size_t>(nodeID)); }
    NodeIDs fillNodes(const CharFrequencies& frequencies);
    void clearNodes() { nodes_.clear(); }

    void buildTree(const NodeIDs& leafs);
    void buildHuffmanDictFromTree(HuffmanDict& dict, const NodeIDs& leafs);

private:
    Nodes nodes_;
    int rootID_ = 0;
    HuffmanDict huffmanDict_;
};

#endif // !HTREE_HPP
