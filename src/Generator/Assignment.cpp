#include "../Builder/TypeCheck.hpp"
#include "CustomOperator.hpp"
#include "Expression.hpp"
#include "UnitFromIdentifier.hpp"

#include "Assignment.hpp"

void assignmentFromTree(generator::GeneratorContext& ctx,
                        const Node& root,
                        const Token& LHSToken,
                        const Token& RHSToken,
                        const Unit& LHS,
                        const Unit& RHS)
{
    Token assignOperator = root.getNthChildTokenFromLast(2);

    if (LHS.isLiteral())
        ctx.console.compileErrorOnToken("Cannot assign to LITERAL", root.children[0].tok);

    if (!isSameType(LHS, RHS))
        ctx.console.typeError(RHSToken, LHS, RHS);

    if (!LHS.isMutable() && root.type == node::ASSIGNMENT)
        ctx.console.compileErrorOnToken("Cannot modify IMMUTABLE variable or parameter", root.children[0].tok);

    if (LHS.isArray() && !assignOperator.isEqualOrLeftArrow())
        ctx.console.compileErrorOnToken("Only EQUAL or LEFT ARROW operator allowed on ARRAY", assignOperator);

    if (assignOperator.isBitwiseOperator() && !LHS.isIntegerType())
        ctx.console.compileErrorOnToken("Bitwise operation not allowed on FLOAT", assignOperator);

    if (assignOperator.getType() == token::EQUAL || assignOperator.getType() == token::WALRUS)
    {
        if (LHS.isPointer() && root.type != node::ASSIGNMENT)
            ctx.console.compileErrorOnToken("Non pointer initialization for POINTER", assignOperator);

        if (LHS.isArray() && RHS.isPointer())
            ctx.console.compileErrorOnToken("Cannot assign POINTER to ARRAY", RHSToken);

        ctx.ir.functionBuilder.unitCopy(LHS, RHS);
    }
    else if (assignOperator.getType() == token::LEFT_ARROW)
    {
        if (!LHS.isPointer())
            ctx.console.compileErrorOnToken("Pointer assignment on a NON POINTER", assignOperator);

        if (!RHS.isValidForPointerAssignment())
            ctx.console.compileErrorOnToken("Invalid expression for POINTER ASSIGNMENT", RHSToken);

        if (LHS.isMutable() && !RHS.isMutable() && !RHS.isPointer() && RHS.isLocal())
            ctx.console.compileErrorOnToken("Cannot assign IMMUTABLE to a pointer", RHSToken);

        ctx.ir.functionBuilder.unitPointerAssign(LHS, RHS);
    }
    else
    {
        ctx.ir.functionBuilder.unitCopy(LHS, binaryOperator(ctx, assignOperator, LHSToken, RHSToken, LHS, RHS));
    }
}

void assignment(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& LHSToken = root.children[0].tok;
    const Token& RHSToken = root.children[2].tok;
    Unit LHS = ordinaryExpression(ctx, root.children[0]);
    Unit RHS = expression(ctx, root.children[2]);

    assignmentFromTree(ctx, root, LHSToken, RHSToken, LHS, RHS);
}
