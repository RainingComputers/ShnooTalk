#include "Module.hpp"
#include "Subscript.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace irgen;
using namespace icode;

bool isVoidFunction(const node::Node& root)
{
    if (!root.isNthChildFromLast(node::IDENTIFIER, 2))
        return true;

    return false;
}

TokenDescriptionPair typeDescriptionFromFunctionNode(ir_generator& ctx, const node::Node& root)
{
    const token::Token& symbolNameToken = root.children[0].tok;

    if (isVoidFunction(root))
        return TokenDescriptionPair(symbolNameToken, ctx.descriptionBuilder.createVoidTypeDescription());

    token::Token dataTypeToken = root.getNthChildTokenFromLast(2);

    return TokenDescriptionPair(symbolNameToken, ctx.descriptionBuilder.createTypeDescription(dataTypeToken));
}

TokenDescriptionPair typeDescriptionFromVarOrParamNode(ir_generator& ctx, const node::Node& root)
{
    const token::Token& symbolNameToken = root.getNthChildToken(0);

    size_t childNodeCounter = 1;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    const token::Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
    TypeDescription typeDescription = ctx.descriptionBuilder.createTypeDescription(dataTypeToken);

    childNodeCounter++;
    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
    {
        LiteralDimensionsIndexPair literalDimensionsIndexPair = getLiteralDimensionFromNode(root, childNodeCounter);

        typeDescription =
          ctx.descriptionBuilder.createArrayTypeDescription(typeDescription, literalDimensionsIndexPair.first);

        childNodeCounter = literalDimensionsIndexPair.second;
    }

    ctx.resetWorkingModule();

    ctx.scope.putInCurrentScope(symbolNameToken);

    return TokenDescriptionPair(symbolNameToken, typeDescription);
}

TokenDescriptionPair typeDescriptionFromNode(ir_generator& ctx, const node::Node& root)
{
    if (root.type == node::FUNCTION)
        return typeDescriptionFromFunctionNode(ctx, root);

    return typeDescriptionFromVarOrParamNode(ctx, root);
}