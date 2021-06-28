#include "Define.hpp"
#include "Enum.hpp"
#include "From.hpp"
#include "Function.hpp"
#include "Global.hpp"
#include "Statement.hpp"
#include "Structure.hpp"

#include "IRGenerator.hpp"

void generator::getUses(generator::GeneratorContext& ctx, const Node& root)
{
    for (Node child : root.children)
    {
        if (child.type == node::USE)
            createUse(ctx, child);
        else
            break;
    }
}

void generateSymbols(generator::GeneratorContext& ctx, const Node& child)
{
    ctx.scope.resetScope();

    switch (child.type)
    {
        case node::USE:
            break;
        case node::FROM:
            createFrom(ctx, child);
            break;
        case node::STRUCT:
            createStructFromNode(ctx, child);
            break;
        case node::FUNCTION:
            createFunctionFromNode(ctx, child);
            break;
        case node::ENUM:
            createEnumFromNode(ctx, child);
            break;
        case node::DEF:
            createDefineFromNode(ctx, child);
            break;
        case node::VAR:
            createGlobalFromNode(ctx, child);
            break;
        default:
            ctx.console.internalBugErrorOnToken(child.tok);
    }
}

void generateFunctions(generator::GeneratorContext& ctx, const Node& child)
{
    const Token& functionNameToken = child.children[0].tok;

    ctx.setWorkingFunction(&ctx.rootModule.functions[functionNameToken.toString()]);

    ctx.scope.resetScope();

    block(ctx,
          child.children.back(),
          false,
          ctx.opBuilder.createLabelOperand(""),
          ctx.opBuilder.createLabelOperand(""),
          ctx.opBuilder.createLabelOperand(""));

    ctx.functionBuilder.terminateFunction(functionNameToken);
}

void generator::generateModule(generator::GeneratorContext& ctx, const Node& root)
{
    for (const Node& child : root.children)
        generateSymbols(ctx, child);

    for (const Node& child : root.children)
        if (child.type == node::FUNCTION)
            generateFunctions(ctx, child);
}