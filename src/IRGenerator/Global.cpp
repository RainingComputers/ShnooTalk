#include "VariableDescriptionFromNode.hpp"

#include "Global.hpp"

void createGlobalFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    TokenDescriptionPair tokenDescriptionPair = variableDescriptionFromNode(ctx, root);

    tokenDescriptionPair.second.setProperty(icode::IS_MUT);

    ctx.descriptionBuilder.createGlobal(tokenDescriptionPair.first, tokenDescriptionPair.second);

    ctx.scope.putInGlobalScope(tokenDescriptionPair.first);
}