#include "Enum.hpp"

void createEnumFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    std::vector<token::Token> enums;

    for (const node::Node& child : root.children)
        enums.push_back(child.tok);

    ctx.descriptionBuilder.createEnum(enums);
}