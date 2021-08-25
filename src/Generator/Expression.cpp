#include "../Builder/TypeCheck.hpp"
#include "Module.hpp"
#include "UnitFromIdentifier.hpp"

#include "Expression.hpp"

using namespace icode;

Unit sizeOf(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();
    setWorkingModuleFromNode(ctx, root, 0);

    int size = ctx.ir.descriptionFinder.getDataTypeSizeFromToken(root.children.back().tok);

    ctx.ir.popWorkingModule();

    return ctx.ir.unitBuilder.unitFromIntLiteral(size);
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
    DataType destinationDataType = stringToDataType(root.tok.toString());

    Unit termToCast = term(ctx, root.children[0]);

    if (termToCast.isArray() || termToCast.isStruct())
        ctx.console.compileErrorOnToken("Cannot cast STRUCT or ARRAY", root.tok);

    return ctx.ir.functionBuilder.castOperator(termToCast, destinationDataType);
}

Unit unaryOperator(generator::GeneratorContext& ctx, const Node& root)
{
    Unit unaryOperatorTerm = term(ctx, root.children[0]);

    if (unaryOperatorTerm.isArray())
        ctx.console.compileErrorOnToken("Unary operator not allowed on ARRAY", root.tok);

    if (unaryOperatorTerm.isStruct())
        ctx.console.compileErrorOnToken("Unary operator not allowed on STRUCT", root.tok);

    if (!unaryOperatorTerm.isIntegerType() && root.tok.getType() == token::NOT)
        ctx.console.compileErrorOnToken("Unary operator NOT not allowed on FLOAT", root.tok);

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
            ctx.console.compileErrorOnToken("Did not expect CONDN NOT", root.tok);
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

    int nodeCounter = setWorkingModuleFromNode(ctx, root, 0);

    if (root.children[nodeCounter].tok.getType() != token::IDENTIFIER)
        ctx.console.compileErrorOnToken("Invalid use of MODULE ACCESS", root.tok);

    Unit result = term(ctx, root.children[nodeCounter]);

    ctx.ir.popWorkingModule();

    return result;
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

    if (root.children.size() != callee.numParameters())
        ctx.console.compileErrorOnToken("Number of parameters don't match", calleeNameToken);

    std::vector<Unit> formalParameters = ctx.ir.descriptionFinder.getFormalParameters(callee);

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

        if (formalParam.isMutableAndPointer() && !actualParam.isUserPointer())
            ctx.console.compileErrorOnToken("Cannot pass EXPRESSION or STRING LITERAL as MUTABLE POINTER",
                                            actualParamToken);

        if (formalParam.isMutableOrPointer() && !actualParam.isValidForPointerAssignment())
            ctx.console.compileErrorOnToken("Cannot pass an EXPRESSION or STRING LITERAL as MUTABLE",
                                            actualParamToken);

        if (formalParam.isMutable() && !actualParam.isMutable())
            ctx.console.compileErrorOnToken("Cannot pass IMMUTABLE as MUTABLE", actualParamToken);

        ctx.ir.functionBuilder.passParameter(calleeNameToken, callee, formalParam, actualParam);
    }

    ctx.ir.popWorkingModule();

    return ctx.ir.functionBuilder.callFunction(calleeNameToken, callee);
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
        case node::UNARY_OPR:
            return unaryOperator(ctx, child);
        case node::EXPRESSION:
            return expression(ctx, child);
        case node::METHODCALL:
        case node::FUNCCALL:
            return functionCall(ctx, child);
        case node::MODULE:
            return switchModuleAndCallTerm(ctx, root);
        case node::SIZEOF:
            return sizeOf(ctx, child);
        default:
            ctx.console.internalBugErrorOnToken(child.tok);
    }
}

Unit stringLiteral(generator::GeneratorContext& ctx, const Node& root)
{
    std::string key = ctx.ir.moduleBuilder.createStringData(root.tok);
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

Instruction tokenToBinaryOperator(const generator::GeneratorContext& ctx, const Token tok)
{
    switch (tok.getType())
    {
        case token::MULTIPLY:
            return MUL;
        case token::DIVIDE:
            return DIV;
        case token::MOD:
            return MOD;
        case token::PLUS:
            return ADD;
        case token::MINUS:
            return SUB;
        case token::RIGHT_SHIFT:
            return RSH;
        case token::LEFT_SHIFT:
            return LSH;
        case token::BITWISE_AND:
            return BWA;
        case token::BITWISE_XOR:
            return BWX;
        case token::BITWISE_OR:
            return BWO;
        case token::CONDN_AND:
        case token::CONDN_OR:
        case token::LESS_THAN:
        case token::LESS_THAN_EQUAL:
        case token::GREATER_THAN:
        case token::GREATER_THAN_EQUAL:
        case token::CONDN_EQUAL:
        case token::CONDN_NOT_EQUAL:
            ctx.console.compileErrorOnToken("Did not expect conditional operator", tok);
        default:
            ctx.console.internalBugErrorOnToken(tok);
    }
}

Unit expression(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.type == node::STR_LITERAL)
        return stringLiteral(ctx, root);

    if (root.type == node::INITLIST)
        return initializerList(ctx, root);

    if (root.type == node::TERM)
        return term(ctx, root);

    if (root.children.size() == 1)
        return expression(ctx, root.children[0]);

    Token expressionOperator = root.children[1].tok;

    Unit LHS = expression(ctx, root.children[0]);

    Unit RHS = expression(ctx, root.children[2]);

    if (LHS.isStruct() || LHS.isArray())
        ctx.console.compileErrorOnToken("Operator not allowed on STRUCT or ARRAY", expressionOperator);

    if (!isSameType(LHS, RHS))
        ctx.console.typeError(root.children[2].tok, LHS, RHS);

    if (expressionOperator.isBitwiseOperation() && !LHS.isIntegerType())
        ctx.console.compileErrorOnToken("Bitwise operations not allowed on FLOAT", expressionOperator);

    Instruction instruction = tokenToBinaryOperator(ctx, expressionOperator);

    return ctx.ir.functionBuilder.binaryOperator(instruction, LHS, RHS);
}
