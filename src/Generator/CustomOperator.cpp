#include "OperatorTokenToInstruction.hpp"
#include "PassParamTypeCheck.hpp"

#include "CustomOperator.hpp"

using namespace icode;

Unit customOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS)
{
    ctx.ir.pushWorkingModule();

    ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromUnit(LHS));

    FunctionDescription callee = ctx.ir.descriptionFinder.getCustomOperatorFunction(binaryOperator, LHS, RHS);

    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

    passParamCheck(ctx, LHS, formalParameters[0], LHSToken);
    passParamCheck(ctx, RHS, formalParameters[1], RHSToken);

    ctx.ir.functionBuilder.passParameter(binaryOperator, callee, formalParameters[0], LHS);
    ctx.ir.functionBuilder.passParameter(binaryOperator, callee, formalParameters[1], RHS);

    ctx.ir.popWorkingModule();

    return ctx.ir.functionBuilder.callFunction(binaryOperator, callee);
}

Unit binaryOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS)
{
    Instruction instruction = tokenToBinaryOperator(ctx, binaryOperator);

    if (LHS.isStruct())
        return customOperator(ctx, binaryOperator, LHSToken, RHSToken, LHS, RHS);

    return ctx.ir.functionBuilder.binaryOperator(instruction, LHS, RHS);
}