#ifndef HTREE_HPP
#define HTREE_HPP

#include <vector>
#include <array>
#include <algorithm>

using BytesBuffer = std::vector<std::uint8_t>;
using BitsBuffer = std::vector<bool>;
using HuffmanDict = std::array<BitsBuffer, 256>;
using CharFrequencies = std::array<std::size_t, 256>;

struct HTreeNode
{
    std::size_t weight = 0;
    std::uint8_t sign = 0;
    int leftNodeID = -1;
    int rightNodeID = -1;
    int parentNodeID = -1;
};

class HTree
{
public: // types
    using Node = HTreeNode;
    using Nodes = std::vector<Node>;
    using NodeIDs = std::vector<int>;

public:
    explicit HTree() = default;
    const HuffmanDict& huffmanDict() const { return huffmanDict_; }
    HuffmanDict& huffmanDict() { return huffmanDict_; }

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

    template<class ByteIt, class BitIt>
    void encodeBytes(ByteIt first, ByteIt last, BitIt outFirst) const
    {
        std::for_each(first, last, [this, outFirst](const std::uint8_t currByte){
            const auto& currBits = huffmanDict_.at(currByte);
            std::copy(std::cbegin(currBits), std::cend(currBits), outFirst);
        });
    }

    template<class BitIt, class ByteIt>
    void decodeBits(BitIt first, BitIt last, ByteIt outFirst) const
    {
        for(; first != last; ++outFirst) {
            int currNodeID = rootID_;
            while(getNode(currNodeID).leftNodeID != -1 && getNode(currNodeID).rightNodeID != -1 && first != last) {
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

    void buildTree(const NodeIDs& freeNodes);
    void buildHuffmanDictFromTree(HuffmanDict& dict, const NodeIDs& leafs);

private:
    Nodes nodes_;
    int rootID_ = 0;
    HuffmanDict huffmanDict_;
};

#endif // !HTREE_HPP
