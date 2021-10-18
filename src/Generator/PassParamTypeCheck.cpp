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
        ctx.console.compileErrorOnToken("Cannot pass EXPRESSION or STRING LITERAL as MUTABLE POINTER",
                                        actualParamToken);

    if (formalParam.isPointer() && actualParam.isLiteral())
        ctx.console.compileErrorOnToken("Cannot pass an LITERAL as MUTABLE or POINTER", actualParamToken);

    if (formalParam.isMutable() && !actualParam.isMutable())
        ctx.console.compileErrorOnToken("Cannot pass IMMUTABLE as MUTABLE", actualParamToken);
}