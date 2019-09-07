#ifndef HTREE_HPP
#define HTREE_HPP

#include <vector>
#include <array>

#include <QDebug>

using BytesBuffer = std::vector<std::uint8_t>;
using BitsBuffer = std::vector<bool>;
using HuffmanDict = std::array<BitsBuffer, 256>;
using CharFrequencies = std::array<int, 256>;

struct HTreeNode
{
    int weight = 0;
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

    template<class It>
    void setBytesRange(It first, It last)
    {
        CharFrequencies frequencies{0};
        CalculateFrequencies(first, last, frequencies);
        const auto leafs = fillNodes(frequencies);
        buildTree(leafs);
        buildHuffmanDictFromTree(huffmanDict_, leafs);
    }

    void setStream(std::istream &stream);

private:
    template<class It>
    static void CalculateFrequencies(It first, It last, CharFrequencies& frequencies)
    {
        for(; first != last; ++first) {
            ++frequencies.at(*first);
        }
    }

    int makeNode() { nodes_.emplace_back(); return static_cast<int>(nodes_.size() - 1); }
    Node& getNode(int nodeID) { return nodes_.at(static_cast<std::size_t>(nodeID)); }
    NodeIDs fillNodes(const CharFrequencies& frequencies);
    void buildTree(const NodeIDs& freeNodes);
    void buildHuffmanDictFromTree(HuffmanDict& dict, const NodeIDs& leafs);

private:
    Nodes nodes_;
    HuffmanDict huffmanDict_;
};

#endif // !HTREE_HPP
