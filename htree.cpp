#include "htree.hpp"

#include <algorithm>
#include <cassert>
#include <queue>

void HTree::setStream(std::istream& stream)
{
    CharFrequencies frequencies{0};
    while (stream) { // calculating frequencies
        std::uint8_t byteChunk = 0;
        stream.read(reinterpret_cast<char*>(&byteChunk), sizeof(byteChunk));
        ++frequencies.at(byteChunk);
    }

    const auto leafs = fillNodes(frequencies);
    buildTree(leafs);
    buildHuffmanDictFromTree(huffmanDict_, leafs);
}

HTree::NodeIDs HTree::fillNodes(const CharFrequencies& frequencies)
{
    NodeIDs leafsIDs;
    for(std::size_t currentSign = 0; currentSign < frequencies.size(); ++currentSign) {
        const int currSignFrequency = frequencies.at(currentSign);
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
