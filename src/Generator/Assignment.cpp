#include "../Builder/TypeCheck.hpp"
#include "Expression.hpp"
#include "UnitFromIdentifier.hpp"

#include "Assignment.hpp"

icode::Instruction assignmentTokenToBinaryOperator(const generator::GeneratorContext& ctx, const Token tok)
{
    switch (tok.getType())
    {
        case token::EQUAL:
            return icode::EQUAL;
        case token::PLUS_EQUAL:
            return icode::ADD;
        case token::MINUS_EQUAL:
            return icode::SUB;
        case token::DIVIDE_EQUAL:
            return icode::DIV;
        case token::MULTIPLY_EQUAL:
            return icode::MUL;
        case token::OR_EQUAL:
            return icode::BWO;
        case token::AND_EQUAL:
            return icode::BWA;
        case token::XOR_EQUAL:
            return icode::BWX;
        default:
            ctx.console.internalBugErrorOnToken(tok);
    }
}

void assignment(generator::GeneratorContext& ctx, const Node& root)
{

    Unit LHS = expression(ctx, root.children[0]);

    Unit RHS = expression(ctx, root.children[2]);

    Token assignOperator = root.getNthChildToken(1);

    if (LHS.op.operandType == icode::LITERAL)
        ctx.console.compileErrorOnToken("Cannot assign to LITERAL", root.children[0].tok);

    if (!isSameType(LHS.type, RHS.type))
        ctx.console.typeError(root.children[2].tok, LHS.type, RHS.type);

    if (!LHS.type.isMutable())
        ctx.console.compileErrorOnToken("Cannot modify IMMUTABLE variable or parameter", root.children[0].tok);

    if ((LHS.type.isStruct() || LHS.type.isArray()) && assignOperator.getType() != token::EQUAL)
        ctx.console.compileErrorOnToken("Only EQUAL operator allowed on STRUCT or ARRAY", assignOperator);

    if (assignOperator.isBitwiseOperation() && !LHS.type.isIntegerType())
        ctx.console.compileErrorOnToken("Bitwise operation not allowed on FLOAT", assignOperator);

    icode::Instruction instruction = assignmentTokenToBinaryOperator(ctx, assignOperator);

    if (assignOperator.getType() == token::EQUAL)
        ctx.functionBuilder.unitCopy(LHS, RHS);
    else
        ctx.functionBuilder.unitCopy(LHS, ctx.functionBuilder.binaryOperator(instruction, LHS, RHS));
}