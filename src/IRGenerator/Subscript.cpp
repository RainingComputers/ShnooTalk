#include "Subscript.hpp"

LiteralDimensionsIndexPair getLiteralDimensionFromNode(const node::node& root, size_t startIndex)
{
    std::vector<int> dimensions;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::SUBSCRIPT, nodeCounter); nodeCounter++)
    {
        const std::string& subscriptString = root.children[nodeCounter].children[0].tok.string;
        dimensions.push_back(std::stoi(subscriptString));
    }

    return LiteralDimensionsIndexPair(dimensions, nodeCounter);
}