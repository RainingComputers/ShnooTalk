#include "Function.hpp"

#include "VariableDescriptionFromNode.hpp"

TokenDescriptionPair getParamFromNode(irgen::ir_generator& ctx, const node::Node& paramNode)
{
    TokenDescriptionPair tokenDescriptionPair = variableDescriptionFromNode(ctx, paramNode);

    bool isMutable = paramNode.isNodeType(node::MUT_PARAM);

    if (isMutable)
        tokenDescriptionPair.second.setProperty(icode::IS_MUT);

    if (isMutable || tokenDescriptionPair.second.isArray() > 0 || tokenDescriptionPair.second.isStruct())
        tokenDescriptionPair.second.setProperty(icode::IS_PTR);

    return tokenDescriptionPair;
}

bool isParamNode(const node::Node& nodeToCheck)
{
    return nodeToCheck.isNodeType(node::PARAM) || nodeToCheck.isNodeType(node::MUT_PARAM);
}

void createFunctionFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    TokenDescriptionPair functionNameAndReturnType = variableDescriptionFromNode(ctx, root);
    const token::Token& nameToken = functionNameAndReturnType.first;
    const icode::VariableDescription& returnType = functionNameAndReturnType.second;

    std::vector<token::Token> paramNames;
    std::vector<icode::VariableDescription> paramTypes;

    for (size_t i = 1; isParamNode(root.children[i]); i += 1)
    {
        TokenDescriptionPair tokenDescriptionPair = getParamFromNode(ctx, root.children[i]);

        paramNames.push_back(tokenDescriptionPair.first);
        paramTypes.push_back(tokenDescriptionPair.second);
    }

    ctx.descriptionBuilder.createFunctionDescription(nameToken, returnType, paramNames, paramTypes);
}