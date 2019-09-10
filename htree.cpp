#include "htree.hpp"

#include <algorithm>
#include <cassert>
#include <queue>

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
