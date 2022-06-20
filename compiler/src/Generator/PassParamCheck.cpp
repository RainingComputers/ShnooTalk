#include "../Builder/TypeCheck.hpp"

#include "PassParamCheck.hpp"

void passParamCheck(const generator::GeneratorContext& ctx,
                    const Unit& formalParam,
                    const Unit& actualParam,
                    const Token& actualParamToken)
{
    if (!isSameType(formalParam, actualParam))
        ctx.console.typeError(actualParamToken, formalParam, actualParam);

    if (formalParam.isUserPointer() && actualParam.isLiteral())
        ctx.console.compileErrorOnToken("Cannot pass literal as mutable or pointer", actualParamToken);

    if (formalParam.isMutable())
    {
        if (formalParam.isUserPointer() && !actualParam.isUserPointer())
            ctx.console.compileErrorOnToken("Cannot pass expression or string literal as mutable pointer",
                                            actualParamToken);

        if (!actualParam.isMutable())
            ctx.console.compileErrorOnToken("Cannot pass immutable as mutable", actualParamToken);
    }

    if (!formalParam.isMutable())
    {
        if (formalParam.isUserPointer() && !actualParam.isMutable() && actualParam.isValue())
            ctx.console.compileErrorOnToken("Cannot pass immutable value to a pointer", actualParamToken);
    }
}
