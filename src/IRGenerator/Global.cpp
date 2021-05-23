#include "DescriptionGenerator/CreateGlobal.hpp"
#include "VariableDescriptionFromNode.hpp"

#include "Global.hpp"

void globalFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    TokenDescriptionPair tokenDescriptionPair = variableDescriptionFromNode(ctx, root);

    tokenDescriptionPair.second.setProperty(icode::IS_MUT);

    createGlobal(ctx, tokenDescriptionPair.first, tokenDescriptionPair.second);
}