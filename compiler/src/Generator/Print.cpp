#include "Expression.hpp"

#include "Print.hpp"

Unit coerceToCharArray(generator::GeneratorContext& ctx, const Token& token, const Unit& unit)
{
    if (ctx.ir.finder.methodExists(unit.type(), "toCharArray"))
    {
        const icode::FunctionDescription charArrayFunc = ctx.ir.finder.getMethod(unit.type(), "toCharArray");

        if (!charArrayFunc.functionReturnType.isSingleDimCharArray())
            ctx.console.compileErrorOnToken("Invalid return type for method 'toCharArray'", token);

        return createCallFunction(ctx, { token }, { unit }, token, charArrayFunc);
    }
    else if (ctx.ir.finder.methodExists(unit.type(), "toString"))
    {
        const icode::FunctionDescription stringFunc = ctx.ir.finder.getMethod(unit.type(), "toString");

        const Unit stringReturn = createCallFunction(ctx, { token }, { unit }, token, stringFunc);

        return coerceToCharArray(ctx, token, stringReturn);
    }

    ctx.console.compileErrorOnToken("Cannot print type without 'toCharArray' or 'toString' method", token);
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