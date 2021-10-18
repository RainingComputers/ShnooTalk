#include "../Builder/TypeCheck.hpp"
#include "BinaryOperator.hpp"
#include "ConditionalExpression.hpp"
#include "Expression.hpp"
#include "OrdinaryExpression.hpp"
#include "UnitFromIdentifier.hpp"

#include "Assignment.hpp"

void assignmentFromTree(generator::GeneratorContext& ctx, const Node& root, const Unit& LHS, const Unit& RHS)
{
    Token assignOperator = root.getNthChildTokenFromLast(2);

    if (LHS.isLiteral())
        ctx.console.compileErrorOnToken("Cannot assign to LITERAL", root.children[0].tok);

    if (!isSameType(LHS, RHS))
        ctx.console.typeError(root.getNthChildTokenFromLast(1), LHS, RHS);

    if (!LHS.isMutable() && root.type == node::ASSIGNMENT)
        ctx.console.compileErrorOnToken("Cannot modify IMMUTABLE variable or parameter", root.children[0].tok);

    if (LHS.isArray() && !assignOperator.isEqualOrLeftArrow())
        ctx.console.compileErrorOnToken("Only EQUAL operator allowed on ARRAY", assignOperator);

    if (assignOperator.isBitwiseOperator() && !LHS.isIntegerType())
        ctx.console.compileErrorOnToken("Bitwise operation not allowed on FLOAT", assignOperator);

    if (assignOperator.getType() == token::EQUAL)
    {
        if (LHS.isPointer() && root.type != node::ASSIGNMENT)
            ctx.console.compileErrorOnToken("Non pointer initialization for POINTER", assignOperator);

        if (LHS.isArray() && RHS.isPointer())
            ctx.console.compileErrorOnToken("Cannot assign POINTER to ARRAY", root.getNthChildTokenFromLast(1));

        ctx.ir.functionBuilder.unitCopy(LHS, RHS);
    }
    else if (assignOperator.getType() == token::LEFT_ARROW)
    {
        if (!LHS.isPointer())
            ctx.console.compileErrorOnToken("Pointer assignment on a NON POINTER", assignOperator);

        if (!RHS.isValidForPointerAssignment())
            ctx.console.compileErrorOnToken("Invalid expression for POINTER ASSIGNMENT",
                                            root.getNthChildTokenFromLast(1));

        ctx.ir.functionBuilder.unitPointerAssign(LHS, RHS);
    }
    else
    {
        ctx.ir.functionBuilder.unitCopy(LHS, binaryOperator(ctx, assignOperator, LHS, RHS));
    }
}

void assignment(generator::GeneratorContext& ctx, const Node& root)
{
    Unit LHS = ordinaryExpression(ctx, root.children[0]);
    Unit RHS = expression(ctx, root.children[2]);

    assignmentFromTree(ctx, root, LHS, RHS);
}
