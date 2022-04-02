#include "../Builder/TypeCheck.hpp"
#include "Expression.hpp"

#include "Return.hpp"

using namespace icode;

void functionReturn(generator::GeneratorContext& ctx, const Node& root)
{
    Unit functionReturn = ctx.ir.functionBuilder.getReturnValueUnit();

    if (root.children.size() != 0)
    {
        Unit returnValue = expression(ctx, root.children[0]);

        if (!isSameType(functionReturn, returnValue))
            ctx.console.typeError(root.children[0].tok, functionReturn, returnValue);

        if (functionReturn.isPointer())
            ctx.ir.functionBuilder.unitPointerAssign(functionReturn, returnValue);
        else
            ctx.ir.functionBuilder.unitCopy(functionReturn, returnValue);
    }
    else if (functionReturn.dtype() != VOID)
        ctx.console.compileErrorOnToken("Ret type is not VOID", root.tok);

    ctx.ir.functionBuilder.createReturnAndCallDeconstructors();
}