#include "Expression.hpp"

#include "Print.hpp"

Unit coerceToCharArray(generator::GeneratorContext& ctx, const Token& token, const Unit& unit)
{
    const std::string toCharArrayHook = "__toCharArray__";
    const std::string toStringHook = "__toString__";

    if (ctx.ir.finder.methodExists(unit.type(), toCharArrayHook))
    {
        const icode::FunctionDescription charArrayFunc = ctx.ir.finder.getMethod(unit.type(), toCharArrayHook);

        if (!charArrayFunc.functionReturnType.isSingleDimCharArray())
            ctx.console.compileErrorOnToken("Invalid return type for method " + toCharArrayHook, token);

        return createCallFunction(ctx, { token }, { unit }, charArrayFunc, token);
    }
    else if (ctx.ir.finder.methodExists(unit.type(), toStringHook))
    {
        const icode::FunctionDescription stringFunc = ctx.ir.finder.getMethod(unit.type(), toStringHook);

        const Unit stringReturn = createCallFunction(ctx, { token }, { unit }, stringFunc, token);

        return coerceToCharArray(ctx, token, stringReturn);
    }

    ctx.console.compileErrorOnToken("Cannot print type without " + toCharArrayHook + " or " + toStringHook + " method",
                                    token);
}

void print(generator::GeneratorContext& ctx, const Node& root)
{
    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        if (child.type == node::SPACE)
        {
            ctx.ir.functionBuilder.noArgumentEntry(icode::SPACE);
        }
        else
        {
            Unit unit = expression(ctx, child);

            if (unit.isStruct() && !unit.isArray())
                unit = coerceToCharArray(ctx, child.tok, unit);

            if ((unit.isArray() && !unit.isChar()) || unit.isMultiDimArray())
                ctx.console.compileErrorOnToken("Cannot print arrays that are not single dimensional char array",
                                                child.tok);

            ctx.ir.functionBuilder.createPrint(unit);
        }

        if (i == root.children.size() - 1 && root.type == node::PRINTLN)
            ctx.ir.functionBuilder.noArgumentEntry(icode::NEWLN);
    }
}