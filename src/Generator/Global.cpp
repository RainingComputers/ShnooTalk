#include "TypeDescriptionFromNode.hpp"

#include "Global.hpp"

using namespace icode;

void createGlobalFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& globalName = root.getNthChildToken(0);

    TypeDescription globalType = typeDescriptionFromNode(ctx, root);

    globalType.becomeMutable();

    ctx.ir.moduleBuilder.createGlobal(globalName, globalType);
}