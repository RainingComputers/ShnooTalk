#include "Subscript.hpp"

LiteralDimensionsIndexPair getLiteralDimensionFromNode(const node::Node& root, size_t startIndex)
{
    std::vector<int> dimensions;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::SUBSCRIPT, nodeCounter); nodeCounter++)
    {
        const int subscriptInt = root.children[nodeCounter].children[0].tok.toInt();
        dimensions.push_back(subscriptInt);
    }

    return LiteralDimensionsIndexPair(dimensions, nodeCounter);
}