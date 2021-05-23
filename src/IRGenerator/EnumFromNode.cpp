#include "DescriptionGenerator/CreateEnum.hpp"

#include "EnumFromNode.hpp"

void enumFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    std::vector<token::Token> enums;

    for (const node::Node& node : root.children)
        enums.push_back(node.tok);

    createEnum(ctx, enums);
}