#include "Module.hpp"
#include "Subscript.hpp"

#include "VariableDescriptionFromNode.hpp"

using namespace irgen;
using namespace icode;

bool isVoidFunction(const node::Node& root)
{
    if (!root.isNthChildFromLast(node::IDENTIFIER, 2))
        return true;

    return false;
}

TokenDescriptionPair variableDescriptionFromFunctionNode(ir_generator& ctx, const node::Node& root)
{
    const token::Token& symbolNameToken = root.children[0].tok;

    if (isVoidFunction(root))
        return TokenDescriptionPair(symbolNameToken, ctx.descriptionBuilder.createVoidVariableDescription());

    token::Token dataTypeToken = root.getNthChildTokenFromLast(2);

    return TokenDescriptionPair(symbolNameToken, ctx.descriptionBuilder.createVariableDescription(dataTypeToken));
}

TokenDescriptionPair variableDescriptionFromVarOrParamNode(ir_generator& ctx, const node::Node& root)
{
    const token::Token& symbolNameToken = root.getNthChildToken(0);

    size_t childNodeCounter = 1;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    const token::Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
    VariableDescription variableDescription = ctx.descriptionBuilder.createVariableDescription(dataTypeToken);

    childNodeCounter++;
    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
    {
        LiteralDimensionsIndexPair literalDimensionsIndexPair = getLiteralDimensionFromNode(root, childNodeCounter);

        variableDescription =
          ctx.descriptionBuilder.createArrayVariableDescription(variableDescription, literalDimensionsIndexPair.first);

        childNodeCounter = literalDimensionsIndexPair.second;
    }

    ctx.resetWorkingModule();

    ctx.scope.putInCurrentScope(symbolNameToken);

    return TokenDescriptionPair(symbolNameToken, variableDescription);
}

TokenDescriptionPair variableDescriptionFromNode(ir_generator& ctx, const node::Node& root)
{
    if (root.type == node::FUNCTION)
        return variableDescriptionFromFunctionNode(ctx, root);

    return variableDescriptionFromVarOrParamNode(ctx, root);
}