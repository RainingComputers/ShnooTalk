#include "../Builder/TypeCheck.hpp"

#include "PassParamTypeCheck.hpp"

void passParamTypeCheck(const generator::GeneratorContext& ctx,
                        const Unit& actualParam,
                        const Unit& formalParam,
                        const Token& actualParamToken)
{
    if (!isSameType(formalParam, actualParam))
        ctx.console.typeError(actualParamToken, formalParam, actualParam);

    if (formalParam.isMutableAndPointer() && !actualParam.isUserPointer())
        ctx.console.compileErrorOnToken("Cannot pass expression or string literal as mutable pointer",
                                        actualParamToken);

    if (formalParam.isPointer() && actualParam.isLiteral())
        ctx.console.compileErrorOnToken("Cannot pass literal as mutable or pointer", actualParamToken);

    if (formalParam.isMutable() && !actualParam.isMutable())
        ctx.console.compileErrorOnToken("Cannot pass immutable as mutable", actualParamToken);
}