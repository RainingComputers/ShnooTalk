#include "../Builder/TypeCheck.hpp"
#include "FunctionCall.hpp"
#include "Module.hpp"
#include "UnitFromIdentifier.hpp"

#include "Expression.hpp"

Unit sizeOf(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.pushWorkingModule();
    setWorkingModuleFromNode(ctx, root, 0);

    int size = ctx.descriptionFinder.getDataTypeSizeFromToken(root.children.back().tok);

    ctx.popWorkingModule();

    return ctx.unitBuilder.unitFromIntLiteral(size);
}

Unit literal(generator::GeneratorContext& ctx, const Node& root)
{
    switch (root.tok.getType())
    {
        case token::INT_LITERAL:
        case token::HEX_LITERAL:
        case token::BIN_LITERAL:
        {
            int literal = std::stoi(root.tok.toString());
            return ctx.unitBuilder.unitFromIntLiteral(literal);
        }
        case token::CHAR_LITERAL:
        {
            char literal = root.tok.toUnescapedString()[0];
            return ctx.unitBuilder.unitFromIntLiteral(literal);
        }
        case token::FLOAT_LITERAL:
        {
            float literal = (float)stof(root.tok.toString());
            return ctx.unitBuilder.unitFromFloatLiteral(literal);
        }
        default:
            ctx.console.internalBugErrorOnToken(root.tok);
    }
}

Unit cast(generator::GeneratorContext& ctx, const Node& root)
{
    icode::DataType destinationDataType = icode::stringToDataType(root.tok.toString());

    Unit termToCast = term(ctx, root.children[0]);

    if (termToCast.type.isArray() || termToCast.type.isStruct())
        ctx.console.compileErrorOnToken("Cannot cast STRUCT or ARRAY", root.tok);

    return ctx.functionBuilder.castOperator(termToCast, destinationDataType);
}

Unit unaryOperator(generator::GeneratorContext& ctx, const Node& root)
{
    Unit unaryOperatorTerm = term(ctx, root.children[0]);

    icode::DataType dtype = unaryOperatorTerm.type.dtype;

    if (unaryOperatorTerm.type.isArray())
        ctx.console.compileErrorOnToken("Unary operator not allowed on ARRAY", root.tok);

    if (unaryOperatorTerm.type.isStruct())
        ctx.console.compileErrorOnToken("Unary operator not allowed on STRUCT", root.tok);

    if (!icode::isInteger(dtype) && root.tok.getType() == token::NOT)
        ctx.console.compileErrorOnToken("Unary operator NOT not allowed on FLOAT", root.tok);

    icode::Instruction instruction;
    switch (root.tok.getType())
    {
        case token::MINUS:
            instruction = icode::UNARY_MINUS;
            break;
        case token::NOT:
            instruction = icode::NOT;
            break;
        case token::CONDN_NOT:
            ctx.console.compileErrorOnToken("Did not expect CONDN NOT", root.tok);
            break;
        default:
            ctx.console.internalBugErrorOnToken(root.tok);
    }

    return ctx.functionBuilder.unaryOperator(instruction, unaryOperatorTerm);
}

Unit switchModuleAndCallTerm(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.pushWorkingModule();
    ctx.resetWorkingModule();

    int nodeCounter = setWorkingModuleFromNode(ctx, root, 0);

    if (root.children[nodeCounter].tok.getType() != token::IDENTIFIER)
        ctx.console.compileErrorOnToken("Invalid use of MODULE ACCESS", root.tok);

    Unit result = term(ctx, root.children[nodeCounter]);

    ctx.popWorkingModule();

    return result;
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
        case node::STRUCT_FUNCCALL:
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

Unit initializerList(generator::GeneratorContext& ctx, const Node& root)
{
    std::vector<Unit> units;

    for (size_t i = 0; i < root.children.size(); i += 1)
    {
        const Node& child = root.children[i];

        units.push_back(expression(ctx, child));

        if (i != 0)
            if (!isSameType(units[i - 1].type, units[i].type))
                ctx.console.typeError(child.tok, units[i - 1].type, units[i].type);
    }

    return ctx.unitBuilder.unitFromUnitList(units);
}

icode::Instruction tokenToBinaryOperator(const generator::GeneratorContext& ctx, const Token tok)
{
    switch (tok.getType())
    {
        case token::MULTIPLY:
            return icode::MUL;
        case token::DIVIDE:
            return icode::DIV;
        case token::MOD:
            return icode::MOD;
        case token::PLUS:
            return icode::ADD;
        case token::MINUS:
            return icode::SUB;
        case token::RIGHT_SHIFT:
            return icode::RSH;
        case token::LEFT_SHIFT:
            return icode::LSH;
        case token::BITWISE_AND:
            return icode::BWA;
        case token::BITWISE_XOR:
            return icode::BWX;
        case token::BITWISE_OR:
            return icode::BWO;
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
        return ctx.strBuilder.createString(root.tok);

    if (root.type == node::INITLIST)
        return initializerList(ctx, root);

    if (root.type == node::TERM)
        return term(ctx, root);

    if (root.children.size() == 1)
        return expression(ctx, root.children[0]);

    Token expressionOperator = root.children[1].tok;

    Unit LHS = expression(ctx, root.children[0]);

    Unit RHS = expression(ctx, root.children[2]);

    if (LHS.type.isStruct() || LHS.type.isArray())
        ctx.console.compileErrorOnToken("Operator not allowed on STRUCT or ARRAY", expressionOperator);

    if (!isSameType(LHS.type, RHS.type))
        ctx.console.typeError(root.children[2].tok, LHS.type, RHS.type);

    if (expressionOperator.isBitwiseOperation() && !LHS.type.isIntegerType())
        ctx.console.compileErrorOnToken("Bitwise operations not allowed on FLOAT", expressionOperator);

    icode::Instruction instruction = tokenToBinaryOperator(ctx, expressionOperator);

    return ctx.functionBuilder.binaryOperator(instruction, LHS, RHS);
}
