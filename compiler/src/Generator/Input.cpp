#include "Expression.hpp"

#include "Input.hpp"

void input(generator::GeneratorContext& ctx, const Node& root)
{
    Unit unit = ordinaryExpression(ctx, root.children[0]);

    if (!unit.isValidForPointerAssignment())
        ctx.console.compileErrorOnToken("Invalid term for input", root.children[0].tok);

    if (unit.isStruct())
        ctx.console.compileErrorOnToken("Cannot get input for a struct", root.children[0].tok);

    if (unit.isMultiDimArray())
        ctx.console.compileErrorOnToken("Cannot get input for multi dimensional array", root.children[0].tok);

    if (unit.isArray() && !unit.isChar())
        ctx.console.compileErrorOnToken("String input requires a char array", root.children[0].tok);

    ctx.ir.functionBuilder.createInput(unit);
}