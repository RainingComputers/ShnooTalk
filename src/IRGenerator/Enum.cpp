#include "Enum.hpp"

void createEnumFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    std::vector<token::Token> enums;

    for (const node::Node& node : root.children)
    {
        enums.push_back(node.tok);
        ctx.scope.putInGlobalScope(node.tok);
    }

    ctx.descriptionBuilder.createEnum(enums);
}