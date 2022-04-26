#include "../Builder/TypeCheck.hpp"
#include "Generic.hpp"
#include "Module.hpp"
#include "OperatorTokenToInstruction.hpp"
#include "PassParamCheck.hpp"
#include "TypeDescriptionFromNode.hpp"
#include "UnitFromIdentifier.hpp"

#include "Expression.hpp"

using namespace icode;

Unit sizeOf(generator::GeneratorContext& ctx, const Node& root)
{
    const icode::TypeDescription& type = typeDescriptionFromNode(ctx, root);

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

    DataType destinationDataType = stringToDataType(root.tok.toString());

    if (destinationDataType == STRUCT)
        ctx.console.compileErrorOnToken("Cannot cast to struct", root.tok);

    if (termToCast.isArray() || termToCast.isStruct())
        ctx.console.compileErrorOnToken("Cannot cast struct or array", root.tok);

    return ctx.ir.functionBuilder.castOperator(termToCast, destinationDataType);
}

Unit addrOperator(generator::GeneratorContext& ctx, const Node& root)
{
    Unit addrTerm = term(ctx, root.children[0]);

    if (!addrTerm.isUserPointer())
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

Unit switchModuleAndCallTerm(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();
    ctx.ir.resetWorkingModule();
    ctx.mm.resetWorkingModule();

    int nodeCounter = setWorkingModuleFromNode(ctx, root, 0);

    const token::TokenType& tokenType = root.children[nodeCounter].tok.getType();

    if (tokenType != token::IDENTIFIER && tokenType != token::GENERATED)
        ctx.console.compileErrorOnToken("Invalid use of namespace access", root.tok);

    Unit result = term(ctx, root.children[nodeCounter]);

    ctx.ir.popWorkingModule();

    return result;
}

Unit createCallFunction(generator::GeneratorContext& ctx,
                        const std::vector<Token>& actualParamTokens,
                        const std::vector<Unit>& actualParams,
                        const Token& calleeNameToken,
                        const FunctionDescription& callee)
{
    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

    if (actualParams.size() != callee.numParameters())
        ctx.console.compileErrorOnToken("Number of parameters don't match", calleeNameToken);

    for (size_t i = 0; i < actualParams.size(); i += 1)
    {
        passParamCheck(ctx, actualParams[i], formalParameters[i], actualParamTokens[i]);
        ctx.ir.functionBuilder.passParameter(calleeNameToken, callee, formalParameters[i], actualParams[i]);
    }

    return ctx.ir.functionBuilder.callFunction(calleeNameToken, callee);
}

Unit functionCall(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();

    Unit firstActualParam;

    if (root.children.size() != 0)
    {
        firstActualParam = expression(ctx, root.children[0]);

        if (root.type == node::METHODCALL)
            ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromUnit(firstActualParam));
    }

    const Token& calleeNameToken = root.tok;
    FunctionDescription callee = ctx.ir.descriptionFinder.getFunction(calleeNameToken);
    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

    ctx.ir.resetWorkingModule();

    std::vector<Token> actualParamTokens;
    std::vector<Unit> actualParams;

    for (size_t i = 0; i < root.children.size(); i++)
    {
        actualParamTokens.push_back(root.children[i].tok);

        if (i == 0)
            actualParams.push_back(firstActualParam);
        else
            actualParams.push_back(expression(ctx, root.children[i]));
    }

    ctx.ir.popWorkingModule();

    return createCallFunction(ctx, actualParamTokens, actualParams, calleeNameToken, callee);
}

Unit genericFunctionCall(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& calleeNameToken = root.tok;

    const std::string& genericModuleName = ctx.mm.getGenericModuleFromToken(calleeNameToken);

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

    const FunctionDescription& callee = intantiateGenericAndGetFunction(ctx,
                                                                        genericModuleName,
                                                                        calleeNameToken,
                                                                        instantiationTypes,
                                                                        instantiationTypeNodes);

    return createCallFunction(ctx, actualParamTokens, actualParams, calleeNameToken, callee);
}

Unit createCallFunctionPremangled(generator::GeneratorContext& ctx,
                                  const std::vector<Token>& actualParamTokens,
                                  const std::vector<Unit>& actualParams,
                                  const std::string& functionName,
                                  const FunctionDescription& callee)
{
    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

    for (size_t i = 0; i < actualParams.size(); i += 1)
    {
        passParamCheck(ctx, actualParams[i], formalParameters[i], actualParamTokens[i]);
        ctx.ir.functionBuilder.passParameterPreMangled(functionName, callee, formalParameters[i], actualParams[i]);
    }

    return ctx.ir.functionBuilder.callFunctionPreMangled(functionName, callee);
}

Unit make(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();

    const TypeDescription& type = typeDescriptionFromNode(ctx, root.children[0]);

    ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromType(type));

    std::vector<Token> actualParamTokens;
    std::vector<Unit> actualParams;

    for (size_t i = 1; i < root.children.size(); i += 1)
    {
        const Token& paramToken = root.children[i].tok;
        Unit param = expression(ctx, root.children[i]);

        actualParams.push_back(param);
        actualParamTokens.push_back(paramToken);

        if (param.isArrayWithFixedDim())
        {
            actualParams.push_back(ctx.ir.unitBuilder.unitFromIntLiteral(param.numElements()));
            actualParamTokens.push_back(paramToken);
        }
    }

    std::pair<std::string, FunctionDescription> constructorNameAndFunction =
        ctx.ir.descriptionFinder.getFunctionByParamTypes(root.tok, type, actualParams);

    const std::string& calleeName = constructorNameAndFunction.first;
    const FunctionDescription& callee = constructorNameAndFunction.second;

    ctx.ir.popWorkingModule();

    return createCallFunctionPremangled(ctx, actualParamTokens, actualParams, calleeName, callee);
}

Unit customOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS)
{
    ctx.ir.pushWorkingModule();

    ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromUnit(LHS));

    std::vector<Token> paramTokens = { LHSToken, RHSToken };
    std::vector<Unit> params = { LHS, RHS };

    if (RHS.isArrayWithFixedDim())
    {
        paramTokens.push_back(RHSToken);
        params.push_back(ctx.ir.unitBuilder.unitFromIntLiteral(RHS.numElements()));
    }

    std::pair<std::string, FunctionDescription> calleeNameAndFunction =
        ctx.ir.descriptionFinder.getCustomOperatorFunction(binaryOperator, params);

    const std::string& calleeName = calleeNameAndFunction.first;
    const FunctionDescription& callee = calleeNameAndFunction.second;

    ctx.ir.popWorkingModule();

    return createCallFunctionPremangled(ctx, paramTokens, params, calleeName, callee);
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

Unit initializerList(generator::GeneratorContext& ctx, const Node& root)
{
    std::vector<Unit> units;

    for (size_t i = 0; i < root.children.size(); i += 1)
    {
        const Node& child = root.children[i];

        units.push_back(expression(ctx, child));

        if (i != 0)
            if (!isSameType(units[i - 1], units[i]))
                ctx.console.typeError(child.tok, units[i - 1], units[i]);
    }

    return ctx.ir.unitBuilder.unitFromUnitList(units);
}

Unit term(generator::GeneratorContext& ctx, const Node& root)
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
            return expression(ctx, child);
        case node::METHODCALL:
        case node::FUNCCALL:
            return functionCall(ctx, child);
        case node::GENERIC_FUNCCALL:
            return genericFunctionCall(ctx, child);
        case node::MODULE:
            return switchModuleAndCallTerm(ctx, root);
        case node::SIZEOF:
            return sizeOf(ctx, child);
        case node::MAKE:
            return make(ctx, child);
        case node::MULTILINE_STR_LITERAL:
            return multilineStringLiteral(ctx, child);
        case node::INITLIST:
            return initializerList(ctx, child);
        default:
            ctx.console.internalBugErrorOnToken(child.tok);
    }
}

Unit ordinaryExpression(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.type == node::INITLIST)
        return initializerList(ctx, root);

    if (root.type == node::TERM)
        return term(ctx, root);

    if (root.children.size() == 1)
        return expression(ctx, root.children[0]);

    Token expressionOperator = root.children[1].tok;

    const Token& LHSToken = root.children[0].tok;
    const Token& RHSToken = root.children[2].tok;
    Unit LHS = expression(ctx, root.children[0]);
    Unit RHS = expression(ctx, root.children[2]);

    if (LHS.isArray())
        ctx.console.compileErrorOnToken("Operator not allowed on array", expressionOperator);

    if (!isSameType(LHS, RHS))
        ctx.console.typeError(root.children[2].tok, LHS, RHS);

    if (expressionOperator.isBitwiseOperator() && LHS.isFloatType())
        ctx.console.compileErrorOnToken("Bitwise operation only allowed on integer types", expressionOperator);

    return binaryOperator(ctx, expressionOperator, LHSToken, RHSToken, LHS, RHS);
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
    Unit RHS = ctx.ir.unitBuilder.unitFromIntLiteral(0);

    if (LHS.isStruct() || LHS.isArray() || !LHS.isIntegerType())
        ctx.console.compileErrorOnToken("Cannot get truth from expression", expressionToken);

    ctx.ir.functionBuilder.compareOperator(icode::GT, LHS, RHS);

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

void relationalExpression(generator::GeneratorContext& ctx,
                          const Token& operatorToken,
                          const Node& root,
                          const icode::Operand& trueLabel,
                          const icode::Operand& falseLabel,
                          bool trueFall)
{
    icode::Instruction opcode = tokenToCompareOperator(ctx, operatorToken);

    const Token& LHSToken = root.children[0].tok;
    const Token& RHSToken = root.children[2].tok;
    Unit LHS = ordinaryExpression(ctx, root.children[0]);
    Unit RHS = ordinaryExpression(ctx, root.children[2]);

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

Unit expression(generator::GeneratorContext& ctx, const Node& root)
{
    if (!root.isConditionalExpression())
        return ordinaryExpression(ctx, root);

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
