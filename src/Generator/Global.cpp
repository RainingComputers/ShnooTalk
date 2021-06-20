#include "TypeDescriptionFromNode.hpp"

#include "Global.hpp"

using namespace icode;

void createGlobalFromNode(irgen::ir_generator& ctx, const Node& root)
{
    const Token& globalName = root.getNthChildToken(0);

    TypeDescription globalType = typeDescriptionFromNode(ctx, root);

    globalType.becomeMutable();

    ctx.moduleBuilder.createGlobal(globalName, globalType);
}