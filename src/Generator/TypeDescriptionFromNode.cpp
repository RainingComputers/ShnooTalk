#include "../Builder/TypeDescriptionUtil.hpp"
#include "Module.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace generator;
using namespace icode;

bool isVoidFunction(const Node& root)
{
    if (!root.isNthChildFromLast(node::IDENTIFIER, 2))
        return true;

    return false;
}

TypeDescription typeDescriptionFromFunctionNode(GeneratorContext& ctx, const Node& root)
{
    if (isVoidFunction(root))
        return ctx.moduleBuilder.createVoidTypeDescription();

    Token dataTypeToken = root.getNthChildTokenFromLast(2);

    return ctx.moduleBuilder.createTypeDescription(dataTypeToken);
}

TypeDescription arrayTypeFromSubscript(GeneratorContext& ctx,
                                       const Node& root,
                                       TypeDescription typeDescription,
                                       size_t startIndex)
{
    std::vector<int> dimensions;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::SUBSCRIPT, nodeCounter); nodeCounter++)
    {
        const int subscriptInt = root.children[nodeCounter].children[0].tok.toInt();
        dimensions.push_back(subscriptInt);
    }

    return createArrayTypeDescription(typeDescription, dimensions, FIXED_DIM);
}

TypeDescription typeDescriptionFromVarOrParamNode(GeneratorContext& ctx, const Node& root)
{
    size_t childNodeCounter = 1;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    const Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
    TypeDescription typeDescription = ctx.moduleBuilder.createTypeDescription(dataTypeToken);

    childNodeCounter++;

    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
        typeDescription = arrayTypeFromSubscript(ctx, root, typeDescription, childNodeCounter);

    ctx.resetWorkingModule();

    return typeDescription;
}

TypeDescription typeDescriptionFromNode(GeneratorContext& ctx, const Node& root)
{
    if (root.type == node::FUNCTION)
        return typeDescriptionFromFunctionNode(ctx, root);

    return typeDescriptionFromVarOrParamNode(ctx, root);
}