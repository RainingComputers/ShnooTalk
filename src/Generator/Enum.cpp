#include "Enum.hpp"

void createEnumFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    std::vector<Token> enums;

    for (const Node& child : root.children)
        enums.push_back(child.tok);

    ctx.ir.moduleBuilder.createEnum(enums);
}