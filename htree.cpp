#include "htree.hpp"

#include <algorithm>
#include <cassert>
#include <queue>

void HTree::setStream(std::istream& inputStream)
{
    // calculating frequencies
    CharFrequencies frequencies{0};
    inputStream.unsetf(std::ios::skipws);
    std::for_each(std::istream_iterator<std::uint8_t>(inputStream), std::istream_iterator<std::uint8_t>(),
    [&frequencies](const std::uint8_t currByte){
        ++frequencies.at(currByte);
    });

    // building tree
    const auto leafs = fillNodes(frequencies);
    buildTree(leafs);
    buildHuffmanDictFromTree(huffmanDict_, leafs);
}

void HTree::setHuffmanDict(const HuffmanDict& dict)
{
    clearNodes();
    const int rootNodeID = makeNode();
    std::copy(std::cbegin(dict), std::cend(dict), std::begin(huffmanDict_));
    rootID_ = rootNodeID;

    for(std::size_t currentSign = 0; currentSign < huffmanDict_.size(); ++currentSign) {
        const auto& bitCodes = huffmanDict_.at(currentSign);

        int currNodeID = rootNodeID;
        for(const bool bitCode : bitCodes) {
            if(bitCode) {
                currNodeID = (getNode(currNodeID).rightNodeID != -1) ? getNode(currNodeID).rightNodeID : makeRightNode(currNodeID);
            }
            else {
                currNodeID = (getNode(currNodeID).leftNodeID != -1) ? getNode(currNodeID).leftNodeID : makeLeftNode(currNodeID);
            }
        }

        getNode(currNodeID).sign = static_cast<std::uint8_t>(currentSign);
    }
}

BitsBuffer HTree::encodeBytes(const BytesBuffer& bytesBuffer) const
{
    BitsBuffer result;
    for(const auto currByte : bytesBuffer) {
        const auto& currBits = huffmanDict_.at(currByte);
        std::copy(std::cbegin(currBits), std::cend(currBits), std::back_inserter(result));
    }
    return result;
}

BytesBuffer HTree::decodeBits(const BitsBuffer& bitsBuffer) const
{
    BytesBuffer result;
    for(std::size_t bitIndex = 0; bitIndex < bitsBuffer.size(); ) {
        int currNodeID = rootID_;
        while(getNode(currNodeID).leftNodeID != -1 && getNode(currNodeID).rightNodeID != -1) {
            currNodeID = bitsBuffer.at(bitIndex) ? getNode(currNodeID).rightNodeID : getNode(currNodeID).leftNodeID;
            ++bitIndex;
        }
        result.push_back(getNode(currNodeID).sign);
    }

    return result;
}

int HTree::makeNode()
{
    nodes_.emplace_back();
    return static_cast<int>(nodes_.size() - 1);
}

int HTree::makeNode(int parentID)
{
    const int newNodeID = makeNode();
    (nodes_.back()).parentNodeID = parentID;
    return newNodeID;
}

int HTree::makeLeftNode(int parentID)
{
    const int newNodeID = makeNode(parentID);
    getNode(parentID).leftNodeID = newNodeID;
    return newNodeID;
}

int HTree::makeRightNode(int parentID)
{
    const int newNodeID = makeNode(parentID);
    getNode(parentID).rightNodeID = newNodeID;
    return newNodeID;
}

HTree::NodeIDs HTree::fillNodes(const CharFrequencies& frequencies)
{
    NodeIDs leafsIDs;
    for(std::size_t currentSign = 0; currentSign < frequencies.size(); ++currentSign) {
        const std::size_t currSignFrequency = frequencies.at(currentSign);
        if(currSignFrequency <= 0) {
            continue;
        }

        const int currNodeID = makeNode();
        auto& lastElem = getNode(currNodeID);
        lastElem.sign = static_cast<std::uint8_t>(currentSign);
        lastElem.weight = currSignFrequency;
        leafsIDs.push_back(currNodeID);
    }
    return leafsIDs;
}

void HTree::buildTree(const NodeIDs& leafs)
{
    assert(!leafs.empty());

    const auto comp = [this](const int left, const int right) {
        return getNode(left).weight > getNode(right).weight;
    };

    std::priority_queue<int, std::vector<int>, decltype(comp)> freeNodes(comp, leafs);
    while(freeNodes.size() > 1) {
        const int leftChildID = freeNodes.top();
        freeNodes.pop();
        const int rightChildID = freeNodes.top();
        freeNodes.pop();

        const int parentID = makeNode();
        auto& parent = getNode(parentID);

        auto& leftChild = getNode(leftChildID);
        auto& rightChild = getNode(rightChildID);

        parent.leftNodeID = leftChildID;
        parent.rightNodeID = rightChildID;
        parent.weight = leftChild.weight + rightChild.weight;

        leftChild.parentNodeID = parentID;
        rightChild.parentNodeID = parentID;

        freeNodes.push(parentID);
    }
    rootID_ = freeNodes.top();
}

void HTree::buildHuffmanDictFromTree(HuffmanDict& dict, const NodeIDs& leafs)
{
    for(const int leafID : leafs) {
        const auto sign = getNode(leafID).sign;
        auto& bits = dict.at(static_cast<std::size_t>(sign));
        for(int parentID = leafID; getNode(parentID).parentNodeID >= 0; parentID = getNode(parentID).parentNodeID) {
            const int parentOfParentID = getNode(parentID).parentNodeID;
            bits.push_back(getNode(parentOfParentID).rightNodeID == parentID);
        }
        std::reverse(std::begin(bits), std::end(bits));
    }
}
