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

    std::vector<Unit> params = { LHS, RHS };

    if (RHS.isArrayWithFixedDim())
        params.push_back(ctx.ir.unitBuilder.unitFromIntLiteral(RHS.numElements()));

    std::pair<std::string, FunctionDescription> calleeNameAndFunction =
        ctx.ir.descriptionFinder.getCustomOperatorFunction(binaryOperator, params);

    const std::string& calleeName = calleeNameAndFunction.first;
    const FunctionDescription& callee = calleeNameAndFunction.second;

    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

    passParamCheck(ctx, LHS, formalParameters[0], LHSToken);
    passParamCheck(ctx, RHS, formalParameters[1], RHSToken);

    for (size_t i = 0; i < params.size(); i += 1)
        ctx.ir.functionBuilder.passParameterPreMangled(calleeName, callee, formalParameters[i], params[i]);

    ctx.ir.popWorkingModule();

    return ctx.ir.functionBuilder.callFunctionPreMangled(calleeName, callee);
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