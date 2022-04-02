#include "Function.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace icode;

TypeDescription getParamType(generator::GeneratorContext& ctx, const Node& paramNode)
{
    TypeDescription paramType = typeDescriptionFromNode(ctx, paramNode);

    if (paramNode.type == node::MUT_PARAM)
        paramType.becomeMutable();

    return paramType;
}

void createFunctionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);
    TypeDescription returnType = typeDescriptionFromNode(ctx, root);

    std::vector<Token> paramNames;
    std::vector<icode::TypeDescription> paramTypes;

    for (size_t i = 1; root.children[i].isParamNode(); i += 1)
    {
        const Token& paramName = root.children[i].getNthChildToken(0);
        TypeDescription paramType = getParamType(ctx, root.children[i]);

        paramNames.push_back(paramName);
        paramTypes.push_back(paramType);

        ctx.scope.putInCurrentScope(paramName);
    }

    if (root.type == node::FUNCTION)
        ctx.ir.moduleBuilder.createFunction(nameToken, returnType, paramNames, paramTypes);
    else if (root.type == node::FUNCTION_EXTERN_C)
        ctx.ir.moduleBuilder.createFunctionExternC(nameToken, returnType, paramNames, paramTypes);
    else if (root.isNthChildFromLast(node::MODULE, 1))
    {
        const Token& moduleNameToken = root.children.back().tok;
        ctx.ir.moduleBuilder.createExternFunctionModule(nameToken, returnType, paramNames, paramTypes, moduleNameToken);
    }
    else
        ctx.ir.moduleBuilder.createExternFunction(nameToken, returnType, paramNames, paramTypes);
}