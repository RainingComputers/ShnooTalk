#include "Expression.hpp"

#include "FunctionCall.hpp"

Unit getActualParam(irgen::ir_generator& ctx, const Node& root, int nodeCounter)
{
    return expression(ctx, root.children[nodeCounter]);
}

Unit functionCall(irgen::ir_generator& ctx, const Node& root)
{
    ctx.pushWorkingModule();

    Unit firstActualParam;

    if (root.children.size() != 0)
    {
        firstActualParam = getActualParam(ctx, root, 0);

        if (root.type == node::STRUCT_FUNCCALL)
            ctx.setWorkingModule(ctx.descriptionFinder.getModuleFromUnit(firstActualParam));
    }

    const Token& calleeNameToken = root.tok;
    icode::FunctionDescription callee = ctx.descriptionFinder.getFunction(calleeNameToken);

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
            actualParam = getActualParam(ctx, root, i);

        if (!icode::isSameType(formalParam.type, actualParam.type))
            ctx.console.typeError(actualParamToken, formalParam.type, actualParam.type);

        if (formalParam.type.isMutable() && !actualParam.op.canPassAsMutable())
            ctx.console.compileErrorOnToken("Cannot pass an EXPRESSION or STRING LITERAL as MUTABLE",
                                            actualParamToken);

        if (formalParam.type.isMutable() && !actualParam.type.isMutable())
            ctx.console.compileErrorOnToken("Cannot pass IMMUTABLE as MUTABLE", actualParamToken);

        ctx.functionBuilder.passParameter(calleeNameToken, callee, formalParam, actualParam);
    }

    ctx.popWorkingModule();

    return ctx.functionBuilder.callFunction(calleeNameToken, callee);
}