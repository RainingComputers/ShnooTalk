#include "../Builder/TypeCheck.hpp"

#include "PassParamTypeCheck.hpp"

void passParamTypeCheck(const generator::GeneratorContext& ctx,
                        const Unit& actualParam,
                        const Unit& formalParam,
                        const Token& actualParamToken)
{
    if (!isSameType(formalParam, actualParam))
        ctx.console.typeError(actualParamToken, formalParam, actualParam);

    if (formalParam.isPointer() && actualParam.isLiteral())
        ctx.console.compileErrorOnToken("Cannot pass literal as mutable or pointer", actualParamToken);

    if (formalParam.isMutable())
    {
        if (formalParam.isPointer() && !actualParam.isUserPointer())
            ctx.console.compileErrorOnToken("Cannot pass expression or string literal as mutable pointer",
                                            actualParamToken);

        if (!actualParam.isMutable())
            ctx.console.compileErrorOnToken("Cannot pass immutable as mutable", actualParamToken);
    }

    if (!formalParam.isMutable())
    {
        if (formalParam.isPointer() && !actualParam.isMutable() && actualParam.isValue())
            ctx.console.compileErrorOnToken("Cannot pass immutable value to a immutable pointer", actualParamToken);
    }
}
