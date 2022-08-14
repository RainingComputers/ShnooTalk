#include "../Builder/TypeCheck.hpp"
#include "../Builder/TypeDescriptionUtil.hpp"
#include "Generic.hpp"
#include "Module.hpp"
#include "OperatorTokenToInstruction.hpp"
#include "PassParamCheck.hpp"
#include "TypeDescriptionFromNode.hpp"
#include "UnitFromIdentifier.hpp"

#include "Expression.hpp"

using namespace icode;

Unit questionMarkOperator(generator::GeneratorContext& ctx, const Node& root);

Unit sizeOf(generator::GeneratorContext& ctx, const Node& root)
{
    const icode::TypeDescription type = typeDescriptionFromNode(ctx, root);

    return ctx.ir.unitBuilder.unitFromIntLiteral(type.size);
}

Unit literal(generator::GeneratorContext& ctx, const Node& root)
{
    switch (root.tok.getType())
    {
        case token::INT_LITERAL:
        case token::HEX_LITERAL:
        case token::BIN_LITERAL:
        {
            long literal = root.tok.toInt();
            return ctx.ir.unitBuilder.unitFromIntLiteral(literal);
        }
        case token::CHAR_LITERAL:
        {
            char literal = root.tok.toUnescapedString()[0];
            return ctx.ir.unitBuilder.unitFromCharLiteral(literal);
        }
        case token::FLOAT_LITERAL:
        {
            double literal = root.tok.toFloat();
            return ctx.ir.unitBuilder.unitFromFloatLiteral(literal);
        }
        default:
            ctx.console.internalBugErrorOnToken(root.tok);
    }
}

Unit cast(generator::GeneratorContext& ctx, const Node& root)
{
    Unit termToCast = term(ctx, root.children[0]);

    DataType destinationDataType = ctx.ir.moduleBuilder.tokenToDataType(root.tok);

    if (destinationDataType == STRUCT)
        ctx.console.compileErrorOnToken("Cannot cast to struct", root.tok);

    if (termToCast.isStructOrArray())
        ctx.console.compileErrorOnToken("Cannot cast struct or array", root.tok);

    return ctx.ir.functionBuilder.castOperator(termToCast, destinationDataType);
}

Unit addrOperator(generator::GeneratorContext& ctx, const Node& root)
{
    Unit addrTerm = term(ctx, root.children[0]);

    if (!addrTerm.isPointer())
        ctx.console.compileErrorOnToken("Cannot apply addr operator to non pointer", root.tok);

    return ctx.ir.functionBuilder.addrOperator(addrTerm);
}

Unit pointerCast(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();

    TypeDescription destinationType = ctx.ir.moduleBuilder.createTypeDescription(root.tok);

    ctx.ir.resetWorkingModule();

    Unit termToCast = term(ctx, root.children[0]);

    if (!termToCast.isValidForPointerAssignment() && !termToCast.isIntegerType())
        ctx.console.compileErrorOnToken("Invalid expression for pointer cast", root.tok);

    if (root.type == node::PTR_ARRAY_CAST)
        destinationType.becomeArrayPointer();
    else
        destinationType.becomePointer();

    ctx.ir.popWorkingModule();

    return ctx.ir.functionBuilder.pointerCastOperator(termToCast, destinationType);
}

Unit unaryOperator(generator::GeneratorContext& ctx, const Node& root)
{
    Unit unaryOperatorTerm = term(ctx, root.children[0]);

    if (unaryOperatorTerm.isArray())
        ctx.console.compileErrorOnToken("Unary operator not allowed on array", root.tok);

    if (unaryOperatorTerm.isStruct())
        ctx.console.compileErrorOnToken("Unary operator not allowed on struct", root.tok);

    if (unaryOperatorTerm.isEnum())
        ctx.console.compileErrorOnToken("Unary operator not allowed on enum", root.tok);

    if (!unaryOperatorTerm.isIntegerType() && root.tok.getType() == token::NOT)
        ctx.console.compileErrorOnToken("Bitwise operation only allowed on integer types", root.tok);

    Instruction instruction;
    switch (root.tok.getType())
    {
        case token::MINUS:
            instruction = UNARY_MINUS;
            break;
        case token::NOT:
            instruction = NOT;
            break;
        case token::CONDN_NOT:
            ctx.console.compileErrorOnToken("Did not expect conditional not", root.tok);
            break;
        default:
            ctx.console.internalBugErrorOnToken(root.tok);
    }

    return ctx.ir.functionBuilder.unaryOperator(instruction, unaryOperatorTerm);
}

Unit switchModuleAndCallTerm(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    ctx.ir.pushWorkingModule();
    ctx.ir.resetWorkingModule();
    ctx.mm.resetWorkingModule();

    int nodeCounter = setWorkingModuleFromNode(ctx, root, 0);

    const token::TokenType tokenType = root.children[nodeCounter].tok.getType();

    if (tokenType != token::IDENTIFIER && tokenType != token::GENERATED)
        ctx.console.compileErrorOnToken("Invalid use of namespace access", root.tok);

    Unit result = termWithHint(ctx, root.children[nodeCounter], typeHint);

    ctx.ir.popWorkingModule();

    return result;
}

void createReturnValue(generator::GeneratorContext& ctx,
                       const Unit& functionReturn,
                       const Unit& returnValue,
                       const Token& errorToken)
{
    if (!isSameType(functionReturn, returnValue))
        ctx.console.typeError(errorToken, functionReturn, returnValue);

    if (functionReturn.isUserPointer())
    {
        if (!returnValue.isValidForPointerAssignment())
            ctx.console.compileErrorOnToken("Invalid expression for pointer return", errorToken);

        if (!returnValue.isMutable() && returnValue.isValue())
            ctx.console.compileErrorOnToken("Cannot return immutable value as pointer", errorToken);

        ctx.ir.functionBuilder.unitPointerAssign(functionReturn, returnValue);
    }
    else
        ctx.ir.functionBuilder.unitCopy(functionReturn, returnValue);
}

void createReturn(generator::GeneratorContext& ctx, const Unit& returnValue, const Token& errorToken)
{
    Unit functionReturn = ctx.ir.functionBuilder.getReturnValueUnit();
    createReturnValue(ctx, functionReturn, returnValue, errorToken);
    ctx.ir.functionBuilder.createReturnAndCallDeconstructors();
}

void functionReturn(generator::GeneratorContext& ctx, const Node& root)
{
    Unit functionReturn = ctx.ir.functionBuilder.getReturnValueUnit();

    if (root.children.size() != 0)
    {
        Unit returnValue = expressionWithHint(ctx, root.children[0], functionReturn.type());
        createReturnValue(ctx, functionReturn, returnValue, root.tok);
    }
    else if (functionReturn.dtype() != VOID)
        ctx.console.compileErrorOnToken("Ret type is not VOID", root.tok);

    ctx.ir.functionBuilder.createReturnAndCallDeconstructors();
}

Unit coerceStringTypes(generator::GeneratorContext& ctx,
                       const Token& token,
                       const Unit& actualParam,
                       const Unit& formalParam)
{
    /* Coerce single dim char arrays (c style strings) if hook is present */
    const std::string coerceArrayHook = "__coerceCharArray__";

    if (actualParam.isSingleDimCharArray() && formalParam.isStruct() &&
        ctx.ir.finder.functionExists(formalParam.type(), coerceArrayHook))
    {
        const FunctionDescription stringFunc = ctx.ir.finder.getMethod(formalParam.type(), coerceArrayHook);
        const Unit lengthParam = ctx.ir.unitBuilder.unitFromIntLiteral(actualParam.numElements());
        return createCallFunction(ctx, { token, token }, { actualParam, lengthParam }, stringFunc, token);
    }

    return actualParam;
}

Unit createCallFunctionMust(generator::GeneratorContext& ctx,
                            const std::vector<Token>& actualParamTokens,
                            const std::vector<Unit>& actualParams,
                            const FunctionDescription& callee)
{
    std::vector<Unit> formalParams = ctx.ir.finder.getFormalParameters(callee);

    for (size_t i = 0; i < formalParams.size(); i += 1)
    {
        Unit actualParam = actualParams[i];
        const Token token = actualParamTokens[i];
        const Unit formalParam = formalParams[i];

        actualParam = coerceStringTypes(ctx, token, actualParam, formalParam);

        passParamCheck(ctx, formalParam, actualParam, token);
        ctx.ir.functionBuilder.passParameter(callee, formalParam, actualParam);
    }

    return ctx.ir.functionBuilder.callFunction(callee);
}

Unit createCallFunction(generator::GeneratorContext& ctx,
                        const std::vector<Token>& actualParamTokens,
                        const std::vector<Unit>& actualParams,
                        const FunctionDescription& callee,
                        const Token& errorToken)
{
    if (actualParams.size() != callee.numParameters())
        ctx.console.compileErrorOnToken("Number of parameters don't match", errorToken);

    return createCallFunctionMust(ctx, actualParamTokens, actualParams, callee);
}

Unit methodCall(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();

    const Token calleeNameToken = root.tok;

    const Unit firstActualParam = expression(ctx, root.children[0]);

    ctx.ir.setWorkingModule(ctx.ir.finder.getModuleFromUnit(firstActualParam));

    if (calleeNameToken.toString() == "__deconstructor__")
        if (!ctx.ir.finder.deconstructorExists(firstActualParam.type()))
            return firstActualParam;

    const FunctionDescription callee = ctx.ir.finder.getFunction(calleeNameToken);
    const std::vector<Unit> formalParameters = ctx.ir.finder.getFormalParameters(callee);

    ctx.ir.resetWorkingModule();

    std::vector<Token> actualParamTokens = { root.children[0].tok };
    std::vector<Unit> actualParams = { firstActualParam };

    for (size_t i = 1; i < root.children.size(); i++)
    {
        actualParamTokens.push_back(root.children[i].tok);
        actualParams.push_back(expressionWithHint(ctx, root.children[i], formalParameters[i].type()));
    }

    if (firstActualParam.isArrayWithFixedDim() && formalParameters.size() == 2 && actualParams.size() == 1)
    {
        actualParamTokens.push_back(actualParamTokens[0]);
        actualParams.push_back(ctx.ir.unitBuilder.unitFromIntLiteral(firstActualParam.numElements()));
    }

    ctx.ir.popWorkingModule();
    return createCallFunction(ctx, actualParamTokens, actualParams, callee, calleeNameToken);
}

Unit functionCall(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();

    const Token calleeNameToken = root.tok;

    FunctionDescription callee = ctx.ir.finder.getFunction(calleeNameToken);
    std::vector<Unit> formalParameters = ctx.ir.finder.getFormalParameters(callee);

    ctx.ir.resetWorkingModule();

    std::vector<Token> actualParamTokens;
    std::vector<Unit> actualParams;

    for (size_t i = 0; i < root.children.size(); i++)
    {
        actualParamTokens.push_back(root.children[i].tok);
        actualParams.push_back(expressionWithHint(ctx, root.children[i], formalParameters[i].type()));
    }

    ctx.ir.popWorkingModule();

    return createCallFunction(ctx, actualParamTokens, actualParams, callee, calleeNameToken);
}

Unit genericFunctionCall(generator::GeneratorContext& ctx, const Node& root)
{
    const Token calleeNameToken = root.tok;

    const std::string genericModuleName = ctx.mm.getGenericModuleFromToken(calleeNameToken);

    std::vector<TypeDescription> instantiationTypes;
    std::vector<Node> instantiationTypeNodes;

    size_t nodeCounter;

    for (nodeCounter = 0; root.children[nodeCounter].type == node::GENERIC_TYPE_PARAM; nodeCounter += 1)
    {
        instantiationTypeNodes.push_back(root.children[nodeCounter]);
        instantiationTypes.push_back(typeDescriptionFromNode(ctx, root.children[nodeCounter]));
    }

    std::vector<Token> actualParamTokens;
    std::vector<Unit> actualParams;

    for (; nodeCounter < root.children.size(); nodeCounter++)
    {
        actualParamTokens.push_back(root.children[nodeCounter].tok);
        actualParams.push_back(expression(ctx, root.children[nodeCounter]));
    }

    const FunctionDescription callee = instantiateGenericAndGetFunction(ctx,
                                                                        genericModuleName,
                                                                        calleeNameToken,
                                                                        instantiationTypes,
                                                                        instantiationTypeNodes);

    return createCallFunction(ctx, actualParamTokens, actualParams, callee, calleeNameToken);
}

Unit genericFunctionCallWithHint(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    ctx.ir.pushWorkingModule();

    ctx.ir.setWorkingModule(ctx.ir.finder.getModuleFromType(typeHint));
    const Unit& result = functionCall(ctx, root);

    ctx.ir.popWorkingModule();

    return result;
}

Unit functionCallWithHint(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    if (!typeHint.isVoid() && !typeHint.isPointer() && ctx.mm.genericExists(root.tok))
        return genericFunctionCallWithHint(ctx, root, typeHint);

    return functionCall(ctx, root);
}

Unit createMake(generator::GeneratorContext& ctx,
                const std::vector<Token>& actualParamTokens,
                const std::vector<Unit>& actualParams,
                const TypeDescription& type,
                const Token& errorToken)
{
    ctx.ir.pushWorkingModule();

    ctx.ir.setWorkingModule(ctx.ir.finder.getModuleFromType(type));

    FunctionDescription constructor = ctx.ir.finder.getFunctionByParamTypes(errorToken, type, actualParams);

    ctx.ir.popWorkingModule();

    return createCallFunctionMust(ctx, actualParamTokens, actualParams, constructor);
}

Unit make(generator::GeneratorContext& ctx, const Node& root)
{

    const TypeDescription type = typeDescriptionFromNode(ctx, root.children[0]);

    std::vector<Token> actualParamTokens;
    std::vector<Unit> actualParams;

    for (size_t i = 1; i < root.children.size(); i += 1)
    {
        const Token paramToken = root.children[i].tok;
        Unit param = expression(ctx, root.children[i]);

        actualParams.push_back(param);
        actualParamTokens.push_back(paramToken);

        if (param.isArrayWithFixedDim())
        {
            actualParams.push_back(ctx.ir.unitBuilder.unitFromIntLiteral(param.numElements()));
            actualParamTokens.push_back(paramToken);
        }
    }

    return createMake(ctx, actualParamTokens, actualParams, type, root.tok);
}

Unit customOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS)
{
    ctx.ir.pushWorkingModule();

    ctx.ir.setWorkingModule(ctx.ir.finder.getModuleFromUnit(LHS));

    std::vector<Token> paramTokens = { LHSToken, RHSToken };
    std::vector<Unit> params = { LHS, RHS };

    if (RHS.isArrayWithFixedDim())
    {
        paramTokens.push_back(RHSToken);
        params.push_back(ctx.ir.unitBuilder.unitFromIntLiteral(RHS.numElements()));
    }

    const std::string customOperatorName = tokenToCustomOperatorString(ctx, binaryOperator);
    FunctionDescription callee = ctx.ir.finder.getCustomOperatorFunction(customOperatorName, params, binaryOperator);

    ctx.ir.popWorkingModule();

    return createCallFunctionMust(ctx, paramTokens, params, callee);
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

Unit multilineStringLiteral(generator::GeneratorContext& ctx, const Node& root)
{
    std::vector<Token> stringTokens;

    for (const Node& child : root.children)
        stringTokens.push_back(child.tok);

    const std::string key = ctx.ir.moduleBuilder.createMultilineStringData(stringTokens);
    return ctx.ir.unitBuilder.unitFromStringDataKey(key);
}

Unit initializerList(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    std::vector<Unit> units;

    for (size_t i = 0; i < root.children.size(); i += 1)
    {
        const Node child = root.children[i];

        units.push_back(expressionWithHint(ctx, child, getElementType(typeHint)));

        if (i != 0)
            if (!isSameType(units[i - 1], units[i]))
                ctx.console.typeError(child.tok, units[i - 1], units[i]);
    }

    return ctx.ir.unitBuilder.unitFromUnitList(units);
}

Unit termWithHint(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    Node child = root.children[0];

    switch (child.type)
    {
        case node::LITERAL:
            return literal(ctx, child);
        case node::IDENTIFIER:
            return getUnitFromIdentifier(ctx, root);
        case node::CAST:
            return cast(ctx, child);
        case node::ADDR:
            return addrOperator(ctx, child);
        case node::PTR_CAST:
        case node::PTR_ARRAY_CAST:
            return pointerCast(ctx, child);
        case node::UNARY_OPR:
            return unaryOperator(ctx, child);
        case node::EXPRESSION:
            return expressionWithHint(ctx, child, typeHint);
        case node::METHODCALL:
            return methodCall(ctx, child);
        case node::FUNCCALL:
            return functionCallWithHint(ctx, child, typeHint);
        case node::GENERIC_FUNCCALL:
            return genericFunctionCall(ctx, child);
        case node::MODULE:
            return switchModuleAndCallTerm(ctx, root, typeHint);
        case node::SIZEOF:
            return sizeOf(ctx, child);
        case node::MAKE:
            return make(ctx, child);
        case node::MULTILINE_STR_LITERAL:
            return multilineStringLiteral(ctx, child);
        case node::INITLIST:
            return initializerList(ctx, child, typeHint);
        case node::QUESTION_OPR:
            return questionMarkOperator(ctx, child);
        default:
            ctx.console.internalBugErrorOnToken(child.tok);
    }
}

Unit term(generator::GeneratorContext& ctx, const Node& root)
{
    return termWithHint(ctx, root, ctx.ir.moduleBuilder.createVoidTypeDescription());
}

Unit ordinaryExpressionWithHint(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    if (root.type == node::INITLIST)
        return initializerList(ctx, root, typeHint);

    if (root.type == node::TERM)
        return termWithHint(ctx, root, typeHint);

    if (root.children.size() == 1)
        return expressionWithHint(ctx, root.children[0], typeHint);

    Token expressionOperator = root.children[1].tok;

    const Token LHSToken = root.children[0].tok;
    const Token RHSToken = root.children[2].tok;
    Unit LHS = expression(ctx, root.children[0]);
    Unit RHS = expressionWithHint(ctx, root.children[2], LHS.type());

    if (LHS.isArray())
        ctx.console.compileErrorOnToken("Operator not allowed on array", expressionOperator);

    if (!LHS.isStruct())
        if (!isSameType(LHS, RHS))
            ctx.console.typeError(root.children[2].tok, LHS, RHS);

    if (expressionOperator.isBitwiseOperator() && LHS.isFloatType())
        ctx.console.compileErrorOnToken("Bitwise operation only allowed on integer types", expressionOperator);

    return binaryOperator(ctx, expressionOperator, LHSToken, RHSToken, LHS, RHS);
}

Unit ordinaryExpression(generator::GeneratorContext& ctx, const Node& root)
{
    return ordinaryExpressionWithHint(ctx, root, ctx.ir.moduleBuilder.createVoidTypeDescription());
}

void conditionalAndOperator(generator::GeneratorContext& ctx,
                            const Token& operatorToken,
                            const Node& root,
                            const icode::Operand& trueLabel,
                            const icode::Operand& falseLabel,
                            bool trueFall)
{
    icode::Operand newTrueLabel = ctx.ir.functionBuilder.createLabel(operatorToken, true, "and");
    icode::Operand newFalseLabel = ctx.ir.functionBuilder.createLabel(operatorToken, false, "and");

    if (trueFall)
        conditionalExpression(ctx, root.children[0], newTrueLabel, falseLabel, true);
    else
        conditionalExpression(ctx, root.children[0], newTrueLabel, newFalseLabel, true);

    conditionalExpression(ctx, root.children[2], trueLabel, falseLabel, trueFall);

    if (!trueFall)
        ctx.ir.functionBuilder.insertLabel(newFalseLabel);
}

void conditionalOrOperator(generator::GeneratorContext& ctx,
                           const Token& operatorToken,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall)
{
    icode::Operand newTrueLabel = ctx.ir.functionBuilder.createLabel(operatorToken, true, "or");
    icode::Operand newFalseLabel = ctx.ir.functionBuilder.createLabel(operatorToken, false, "or");

    if (!trueFall)
        conditionalExpression(ctx, root.children[0], trueLabel, newFalseLabel, false);
    else
        conditionalExpression(ctx, root.children[0], newTrueLabel, newFalseLabel, false);

    conditionalExpression(ctx, root.children[2], trueLabel, falseLabel, trueFall);

    if (trueFall)
        ctx.ir.functionBuilder.insertLabel(newTrueLabel);
}

void createJumps(generator::GeneratorContext& ctx,
                 const icode::Operand& trueLabel,
                 const icode::Operand& falseLabel,
                 bool trueFall)
{
    if (!trueFall)
        ctx.ir.functionBuilder.createBranch(icode::IF_TRUE_GOTO, trueLabel);
    else
        ctx.ir.functionBuilder.createBranch(icode::IF_FALSE_GOTO, falseLabel);
}

void truthyOperator(generator::GeneratorContext& ctx,
                    const Unit& LHS,
                    const Token& expressionToken,
                    const icode::Operand& trueLabel,
                    const icode::Operand& falseLabel,
                    bool trueFall)
{
    const std::string isNonZeroHook = "__isNonZero__";

    Unit RHS = ctx.ir.unitBuilder.unitFromIntLiteral(0);

    if (LHS.isArray() || (!LHS.isIntegerType() && !LHS.isStruct()))
        ctx.console.compileErrorOnToken("Cannot get truth from expression", expressionToken);

    if (LHS.isStruct())
    {
        const icode::FunctionDescription isNonZeroFunc =
            ctx.ir.finder.getMethodFromUnit(LHS, isNonZeroHook, expressionToken);

        const Unit isNonZeroFuncRetVal =
            createCallFunction(ctx, { expressionToken }, { LHS }, isNonZeroFunc, expressionToken);

        if (!isNonZeroFuncRetVal.isIntegerType() || isNonZeroFuncRetVal.isArray())
            ctx.console.compileErrorOnToken("Invalid return type for " + isNonZeroHook + " function", expressionToken);

        ctx.ir.functionBuilder.compareOperator(icode::GT, isNonZeroFuncRetVal, RHS);
    }
    else
    {
        ctx.ir.functionBuilder.compareOperator(icode::GT, LHS, RHS);
    }

    createJumps(ctx, trueLabel, falseLabel, trueFall);
}

void truthyExpression(generator::GeneratorContext& ctx,
                      const Node& root,
                      const icode::Operand& trueLabel,
                      const icode::Operand& falseLabel,
                      bool trueFall)
{
    Unit LHS = ordinaryExpression(ctx, root);
    truthyOperator(ctx, LHS, root.tok, trueLabel, falseLabel, trueFall);
}

void createQuestionMarkReturn(generator::GeneratorContext& ctx,
                              const Unit& term,
                              const FunctionDescription& errorFunc,
                              const FunctionDescription& constructorFunc,
                              const Token& errorToken)
{
    const Unit errorFuncRet = createCallFunctionMust(ctx, { errorToken }, { term }, errorFunc);

    Unit constructorFuncRet;

    if (constructorFunc.numParameters() == 0)
        constructorFuncRet = createCallFunctionMust(ctx, {}, {}, constructorFunc);
    else
        constructorFuncRet = createCallFunctionMust(ctx, { errorToken }, { errorFuncRet }, constructorFunc);

    createReturn(ctx, constructorFuncRet, errorToken);
}

Unit questionMarkOperator(generator::GeneratorContext& ctx, const Node& root)
{
    const Unit term = expression(ctx, root.children[0]);
    const Token expressionToken = root.children[0].tok;
    const TypeDescription returnType = ctx.ir.functionBuilder.getReturnValueUnit().type();

    const std::string continueHook = "__questionMarkContinue__";
    const std::string unwrapHook = "__questionMarkUnwrap__";
    const std::string errorHook = "__questionMarkError__";
    const std::string constructorHook = "__questionMarkConstruct__";

    if (!ctx.ir.finder.functionExists(returnType, constructorHook))
        ctx.console.compileErrorOnToken("Cannot use ? operator without " + constructorHook +
                                            " function for return type",
                                        root.tok);

    const FunctionDescription continueFunc = ctx.ir.finder.getMethodFromUnit(term, continueHook, root.tok);
    const FunctionDescription errorFunc = ctx.ir.finder.getMethodFromUnit(term, errorHook, root.tok);
    const FunctionDescription constructorFunc = ctx.ir.finder.getMethod(returnType, constructorHook);
    const FunctionDescription unwrapFunc = ctx.ir.finder.getMethodFromUnit(term, unwrapHook, root.tok);

    const icode::Operand falseLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "question_op");
    const icode::Operand trueLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "question_op");

    const Unit continueFuncRet = createCallFunctionMust(ctx, { expressionToken }, { term }, continueFunc);
    truthyOperator(ctx, continueFuncRet, root.tok, trueLabel, falseLabel, false);

    createQuestionMarkReturn(ctx, term, errorFunc, constructorFunc, expressionToken);

    ctx.ir.functionBuilder.insertLabel(trueLabel);

    return createCallFunctionMust(ctx, { expressionToken }, { term }, unwrapFunc);
}

void relationalExpression(generator::GeneratorContext& ctx,
                          const Token& operatorToken,
                          const Node& root,
                          const icode::Operand& trueLabel,
                          const icode::Operand& falseLabel,
                          bool trueFall)
{
    const Token LHSToken = root.children[0].tok;
    const Token RHSToken = root.children[2].tok;
    Unit LHS = ordinaryExpression(ctx, root.children[0]);
    Unit RHS = ordinaryExpression(ctx, root.children[2]);

    if (operatorToken.getType() == token::IN)
        std::swap(LHS, RHS);

    if (LHS.isArray())
        ctx.console.compileErrorOnToken("Cannot compare arrays", operatorToken);

    if (!LHS.isStruct())
        if (!isSameType(LHS, RHS))
            ctx.console.typeError(root.children[2].tok, LHS, RHS);

    if (LHS.isStruct())
    {
        const Unit customOperatorResult = customOperator(ctx, operatorToken, LHSToken, RHSToken, LHS, RHS);
        truthyOperator(ctx, customOperatorResult, operatorToken, trueLabel, falseLabel, trueFall);
    }
    else
    {
        icode::Instruction opcode = tokenToCompareOperator(ctx, operatorToken);

        ctx.ir.functionBuilder.compareOperator(opcode, LHS, RHS);
        createJumps(ctx, trueLabel, falseLabel, trueFall);
    }
}

void conditionalTerm(generator::GeneratorContext& ctx,
                     const Node& root,
                     const icode::Operand& trueLabel,
                     const icode::Operand& falseLabel,
                     bool trueFall)
{
    if (root.children[0].tok.getType() == token::CONDN_NOT)
        conditionalExpression(ctx, root.children[0].children[0], falseLabel, trueLabel, !trueFall);
    else if (root.children[0].type == node::EXPRESSION)
        conditionalExpression(ctx, root.children[0], trueLabel, falseLabel, trueFall);
    else
        truthyExpression(ctx, root, trueLabel, falseLabel, trueFall);
}

void conditionalExpression(generator::GeneratorContext& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall)
{
    if (root.type == node::TERM)
        conditionalTerm(ctx, root, trueLabel, falseLabel, trueFall);
    else if (root.children.size() == 1)
        conditionalExpression(ctx, root.children[0], trueLabel, falseLabel, trueFall);
    else
    {
        Token operatorToken = root.children[1].tok;

        if (operatorToken.getType() == token::CONDN_AND)
            conditionalAndOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        else if (operatorToken.getType() == token::CONDN_OR)
            conditionalOrOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        else if (operatorToken.isConditionalOperator())
            relationalExpression(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        else
            truthyExpression(ctx, root, trueLabel, falseLabel, trueFall);
    }
}

Unit expressionWithHint(generator::GeneratorContext& ctx, const Node& root, const TypeDescription& typeHint)
{
    if (!root.isConditionalExpression())
        return ordinaryExpressionWithHint(ctx, root, typeHint);

    Unit LHS = ctx.ir.functionBuilder.createTemp(icode::AUTO_INT);

    icode::Operand trueLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "condn_exp");
    icode::Operand falseLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "condn_exp");

    icode::Operand endLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "condn_exp_end");

    conditionalExpression(ctx, root, trueLabel, falseLabel, true);

    ctx.ir.functionBuilder.unitCopy(LHS, ctx.ir.unitBuilder.unitFromIntLiteral(1));
    ctx.ir.functionBuilder.createBranch(icode::GOTO, endLabel);

    ctx.ir.functionBuilder.insertLabel(falseLabel);

    ctx.ir.functionBuilder.unitCopy(LHS, ctx.ir.unitBuilder.unitFromIntLiteral(0));

    ctx.ir.functionBuilder.insertLabel(endLabel);

    return LHS;
}

Unit expression(generator::GeneratorContext& ctx, const Node& root)
{
    return expressionWithHint(ctx, root, ctx.ir.moduleBuilder.createVoidTypeDescription());
}
