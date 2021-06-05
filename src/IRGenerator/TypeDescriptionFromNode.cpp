#include "Module.hpp"
#include "Subscript.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace irgen;
using namespace icode;

bool isVoidFunction(const Node& root)
{
    if (!root.isNthChildFromLast(node::IDENTIFIER, 2))
        return true;

    return false;
}

TypeDescription typeDescriptionFromFunctionNode(ir_generator& ctx, const Node& root)
{
    if (isVoidFunction(root))
        return ctx.descriptionBuilder.createVoidTypeDescription();

    Token dataTypeToken = root.getNthChildTokenFromLast(2);

    return ctx.descriptionBuilder.createTypeDescription(dataTypeToken);
}

TypeDescription typeDescriptionFromVarOrParamNode(ir_generator& ctx, const Node& root)
{
    size_t childNodeCounter = 1;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    const Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
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

    return typeDescription;
}

TypeDescription typeDescriptionFromNode(ir_generator& ctx, const Node& root)
{
    if (root.type == node::FUNCTION)
        return typeDescriptionFromFunctionNode(ctx, root);

    return typeDescriptionFromVarOrParamNode(ctx, root);
}