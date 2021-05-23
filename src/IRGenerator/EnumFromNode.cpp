#include "DescriptionGenerator/CreateEnum.hpp"

#include "EnumFromNode.hpp"

void enumFromNode(irgen::ir_generator& ctx, const node::node& root)
{
    std::vector<token::Token> enums;

    for(const node::node& node : root.children)
        enums.push_back(node.tok);

    createEnum(ctx, enums);
}