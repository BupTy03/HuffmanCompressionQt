#ifndef HTREE_HPP
#define HTREE_HPP

#include <vector>
#include <array>

#include <QDebug>

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

    void setStream(std::istream& inputStream);
    void setHuffmanDict(const HuffmanDict& dict);

    BitsBuffer encodeBytes(const BytesBuffer& bytesBuffer) const;
    BytesBuffer decodeBits(const BitsBuffer& bitsBuffer) const;

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
