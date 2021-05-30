#include "TypeDescriptionFromNode.hpp"

#include "Global.hpp"

using namespace icode;

void createGlobalFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    const token::Token& globalName = root.getNthChildToken(0);
    
    TypeDescription globalType = typeDescriptionFromNode(ctx, root);
    globalType.setProperty(IS_GLOBAL);

    globalType.setProperty(icode::IS_MUT);

    ctx.descriptionBuilder.createGlobal(globalName, globalType);
}