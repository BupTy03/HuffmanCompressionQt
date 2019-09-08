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

    template<class It>
    void setBytesRange(It first, It last)
    {
        CharFrequencies frequencies{0};
        CalculateFrequencies(first, last, frequencies);
        for(std::size_t i = 0; i < frequencies.size(); ++i) {
            if(frequencies.at(i) > 0) {
                qDebug() << "Char: '" << (char)i << "' = " << frequencies.at(i);
            }
        }
//        const auto leafs = fillNodes(frequencies);
//        buildTree(leafs);
//        buildHuffmanDictFromTree(huffmanDict_, leafs);
    }

    void setStream(std::istream& stream);
    void setHuffmanDict(const HuffmanDict& dict);

    BitsBuffer encodeBytes(const BytesBuffer& bytesBuffer) const;
    BytesBuffer decodeBits(const BitsBuffer& bitsBuffer) const;

    void compress(std::istream& from, std::ostream& to);
    void decompress(std::istream& from, std::ostream& to);

private:
    template<class It>
    static void CalculateFrequencies(It first, It last, CharFrequencies& frequencies)
    {
        for(; first != last; ++first) {
            ++frequencies.at(*first);
        }
    }

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
