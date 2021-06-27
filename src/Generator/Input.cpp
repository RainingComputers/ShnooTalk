#include "Expression.hpp"

#include "Input.hpp"

void input(irgen::ir_generator& ctx, const Node& root)
{
    Unit unit = expression(ctx, root.children[0]);

    if (unit.op.isInvalidForInput())
        ctx.console.compileErrorOnToken("Invalid term for INPUT", root.children[0].tok);

    if (unit.type.isStruct())
        ctx.console.compileErrorOnToken("Cannot INPUT STRUCT", root.children[0].tok);

    if (unit.type.dimensions.size() > 1)
        ctx.console.compileErrorOnToken("Cannot INPUT more than 1D ARRAY", root.children[0].tok);

    if (unit.type.dimensions.size() == 1 && unit.op.dtype != icode::UI8)
        ctx.console.compileErrorOnToken("String input requires 1D CHAR ARRAY", root.children[0].tok);

    ctx.functionBuilder.createInput(unit);
}