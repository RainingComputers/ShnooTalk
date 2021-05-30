#include "Function.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace icode;

TypeDescription getParamType(irgen::ir_generator& ctx, const node::Node& paramNode)
{
    TypeDescription paramType = typeDescriptionFromNode(ctx, paramNode);

    bool isMutable = paramNode.isNodeType(node::MUT_PARAM);

    if (isMutable)
        paramType.setProperty(icode::IS_MUT);

    if (isMutable || paramType.isArray() > 0 || paramType.isStruct())
        paramType.setProperty(icode::IS_PTR);

    paramType.setProperty(icode::IS_PARAM);

    return paramType;
}

bool isParamNode(const node::Node& nodeToCheck)
{
    return nodeToCheck.isNodeType(node::PARAM) || nodeToCheck.isNodeType(node::MUT_PARAM);
}

void createFunctionFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    const token::Token& nameToken = root.getNthChildToken(0);
    TypeDescription returnType = typeDescriptionFromNode(ctx, root);

    std::vector<token::Token> paramNames;
    std::vector<icode::TypeDescription> paramTypes;

    for (size_t i = 1; isParamNode(root.children[i]); i += 1)
    {
        const token::Token& paramName = root.children[i].getNthChildToken(0);
        TypeDescription paramType = getParamType(ctx, root.children[i]);

        paramNames.push_back(paramName);
        paramTypes.push_back(paramType);
    
        ctx.scope.putInCurrentScope(paramName);
    }

    ctx.descriptionBuilder.createFunctionDescription(nameToken, returnType, paramNames, paramTypes);
}