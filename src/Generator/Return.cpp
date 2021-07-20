#include "../Builder/TypeCheck.hpp"
#include "Expression.hpp"

#include "Return.hpp"

using namespace icode;

void functionReturn(generator::GeneratorContext& ctx, const Node& root)
{
    Unit returnTypeUnit = ctx.ir.descriptionFinder.getReturnType();

    if (root.children.size() != 0)
    {
        Unit returnValue = expression(ctx, root.children[0]);

        if (!isSameType(returnTypeUnit, returnValue))
            ctx.console.typeError(root.children[0].tok, returnTypeUnit, returnValue);

        Unit returnPointer = ctx.ir.functionBuilder.getReturnPointerUnit();

        ctx.ir.functionBuilder.unitCopy(returnPointer, returnValue);
    }
    else if (returnTypeUnit.dtype() != VOID)
        ctx.console.compileErrorOnToken("Ret type is not VOID", root.tok);

    ctx.ir.functionBuilder.noArgumentEntry(RET);
}