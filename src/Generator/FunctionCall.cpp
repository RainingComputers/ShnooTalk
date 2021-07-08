#include "../Builder/TypeCheck.hpp"
#include "Expression.hpp"

#include "FunctionCall.hpp"

using namespace icode;

Unit functionCall(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.pushWorkingModule();

    Unit firstActualParam;

    if (root.children.size() != 0)
    {
        firstActualParam = expression(ctx, root.children[0]);

        if (root.type == node::STRUCT_FUNCCALL)
            ctx.setWorkingModule(ctx.descriptionFinder.getModuleFromUnit(firstActualParam));
    }

    const Token& calleeNameToken = root.tok;
    FunctionDescription callee = ctx.descriptionFinder.getFunction(calleeNameToken);

    if (root.children.size() != callee.numParameters())
        ctx.console.compileErrorOnToken("Number of parameters don't match", calleeNameToken);

    std::vector<Unit> formalParameters = ctx.descriptionFinder.getFormalParameters(callee);

    for (size_t i = 0; i < root.children.size(); i++)
    {
        Unit formalParam = formalParameters[i];

        Unit actualParam;
        const Token& actualParamToken = root.children[i].tok;

        if (i == 0)
            actualParam = firstActualParam;
        else
            actualParam = expression(ctx, root.children[i]);

        if (!isSameType(formalParam, actualParam))
            ctx.console.typeError(actualParamToken, formalParam, actualParam);

        if (formalParam.isMutable() && !actualParam.canPassAsMutable())
            ctx.console.compileErrorOnToken("Cannot pass an EXPRESSION or STRING LITERAL as MUTABLE",
                                            actualParamToken);

        if (formalParam.isMutable() && !actualParam.isMutable())
            ctx.console.compileErrorOnToken("Cannot pass IMMUTABLE as MUTABLE", actualParamToken);

        ctx.functionBuilder.passParameter(calleeNameToken, callee, formalParam, actualParam);
    }

    ctx.popWorkingModule();

    return ctx.functionBuilder.callFunction(calleeNameToken, callee);
}

void functionReturn(generator::GeneratorContext& ctx, const Node& root)
{
    Unit returnTypeUnit = ctx.descriptionFinder.getReturnType();

    if (root.children.size() != 0)
    {
        Unit returnValue = expression(ctx, root.children[0]);

        if (!isSameType(returnTypeUnit, returnValue))
            ctx.console.typeError(root.children[0].tok, returnTypeUnit, returnValue);

        Unit returnPointer = ctx.functionBuilder.getReturnPointerUnit();

        ctx.functionBuilder.unitCopy(returnPointer, returnValue);
    }
    else if (returnTypeUnit.dtype() != VOID)
        ctx.console.compileErrorOnToken("Ret type is not VOID", root.tok);

    ctx.functionBuilder.noArgumentEntry(RET);
}