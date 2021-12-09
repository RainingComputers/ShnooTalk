#include "../Builder/TypeDescriptionUtil.hpp"
#include "Module.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace generator;
using namespace icode;

TypeDescription arrayTypeFromSubscript(const Node& root, const TypeDescription& typeDescription, size_t startIndex)
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

bool isGenericTypeNode(GeneratorContext& ctx, const Node& root)
{
    return root.isNthChildFromLast(node::GENERIC, 1);
}

TypeDescription typeDescriptionFromNode(GeneratorContext& ctx, const Node& root)
{
    size_t childNodeCounter = 1;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    if (!root.isNthChild(node::IDENTIFIER, childNodeCounter))
        return ctx.ir.moduleBuilder.createVoidTypeDescription();

    const Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
    TypeDescription typeDescription = ctx.ir.moduleBuilder.createTypeDescription(dataTypeToken);

    childNodeCounter++;

    if (root.isNthChild(node::POINTER_STAR, childNodeCounter))
        typeDescription.becomePointer();

    if (root.isNthChild(node::EMPTY_SUBSCRIPT, childNodeCounter))
        typeDescription.becomeArrayPointer();

    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
        typeDescription = arrayTypeFromSubscript(root, typeDescription, childNodeCounter);

    ctx.ir.resetWorkingModule();

    return typeDescription;
}