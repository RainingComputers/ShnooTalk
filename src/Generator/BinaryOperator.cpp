#include "OperatorTokenToInstruction.hpp"
#include "PassParamTypeCheck.hpp"

#include "BinaryOperator.hpp"

using namespace icode;

Unit customBinaryOperator(generator::GeneratorContext& ctx,
                          const Token& binaryOperator,
                          const Unit& LHS,
                          const Unit& RHS)
{
    ctx.ir.pushWorkingModule();

    ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromUnit(LHS));

    FunctionDescription callee = ctx.ir.descriptionFinder.getFunction(binaryOperator);

    if (callee.numParameters() != 2)
        ctx.console.compileErrorOnToken("Invalid number of parameters for OPERATOR FUNCTION", binaryOperator);

    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

    passParamTypeCheck(ctx, LHS, formalParameters[0], binaryOperator);
    passParamTypeCheck(ctx, RHS, formalParameters[1], binaryOperator);

    ctx.ir.functionBuilder.passParameter(binaryOperator, callee, formalParameters[0], LHS);
    ctx.ir.functionBuilder.passParameter(binaryOperator, callee, formalParameters[1], RHS);

    ctx.ir.popWorkingModule();

    return ctx.ir.functionBuilder.callFunction(binaryOperator, callee);
}

Unit binaryOperator(generator::GeneratorContext& ctx, const Token& binaryOperator, const Unit& LHS, const Unit& RHS)
{
    Instruction instruction = tokenToBinaryOperator(ctx, binaryOperator);

    if (LHS.isStruct())
        return customBinaryOperator(ctx, binaryOperator, LHS, RHS);

    return ctx.ir.functionBuilder.binaryOperator(instruction, LHS, RHS);
}