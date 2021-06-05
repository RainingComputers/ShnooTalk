#include "Enum.hpp"

void createEnumFromNode(irgen::ir_generator& ctx, const Node& root)
{
    std::vector<Token> enums;

    for (const Node& child : root.children)
        enums.push_back(child.tok);

    ctx.descriptionBuilder.createEnum(enums);
}