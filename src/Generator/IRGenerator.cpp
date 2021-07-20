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

void generateSymbol(generator::GeneratorContext& ctx, const Node& child)
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

void generateFunction(generator::GeneratorContext& ctx, const Node& child)
{
    const Token& functionNameToken = child.children[0].tok;

    ctx.ir.setWorkingFunction(functionNameToken);

    ctx.scope.resetScope();

    block(ctx,
          child.children.back(),
          false,
          ctx.ir.opBuilder.createLabelOperand(""),
          ctx.ir.opBuilder.createLabelOperand(""),
          ctx.ir.opBuilder.createLabelOperand(""));

    ctx.ir.functionBuilder.terminateFunction(functionNameToken);
}

void generator::generateModule(generator::GeneratorContext& ctx, const Node& root)
{
    for (const Node& child : root.children)
        generateSymbol(ctx, child);

    for (const Node& child : root.children)
        if (child.type == node::FUNCTION)
            generateFunction(ctx, child);
}