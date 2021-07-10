#include "Function.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace icode;

TypeDescription getParamType(generator::GeneratorContext& ctx, const Node& paramNode)
{
    TypeDescription paramType = typeDescriptionFromNode(ctx, paramNode);

    bool isMutable = paramNode.type == node::MUT_PARAM;

    if (isMutable)
        paramType.becomeMutable();

    if (isMutable || paramType.isArray() > 0 || paramType.isStruct())
        paramType.becomePointer();

    return paramType;
}

bool isParamNode(const Node& nodeToCheck)
{
    return nodeToCheck.type == node::PARAM || nodeToCheck.type == node::MUT_PARAM;
}

void createFunctionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);
    TypeDescription returnType = typeDescriptionFromNode(ctx, root);

    std::vector<Token> paramNames;
    std::vector<icode::TypeDescription> paramTypes;

    for (size_t i = 1; isParamNode(root.children[i]); i += 1)
    {
        const Token& paramName = root.children[i].getNthChildToken(0);
        TypeDescription paramType = getParamType(ctx, root.children[i]);

        paramNames.push_back(paramName);
        paramTypes.push_back(paramType);

        ctx.scope.putInCurrentScope(paramName);
    }

    ctx.moduleBuilder.createFunctionDescription(nameToken, returnType, paramNames, paramTypes);
}