#include "../Builder/TypeCheck.hpp"
#include "CustomOperator.hpp"
#include "Expression.hpp"
#include "UnitFromIdentifier.hpp"

#include "Assignment.hpp"

void assignmentFromTree(generator::GeneratorContext& ctx,
                        const node::NodeType rootNodeType,
                        const Token& assignOperator,
                        const Token& LHSToken,
                        const Token& RHSToken,
                        const Unit& LHS,
                        const Unit& RHS)
{
    bool assignmentNode = rootNodeType == node::ASSIGNMENT || rootNodeType == node::DESTRUCTURED_ASSIGNMENT;

    if (LHS.isLiteral())
        ctx.console.compileErrorOnToken("Cannot assign to literal", LHSToken);

    if (!LHS.isStruct() || assignOperator.isEqualOrLeftArrow())
        if (!isSameType(LHS, RHS))
            ctx.console.typeError(RHSToken, LHS, RHS);

    if (!LHS.isMutable() && assignmentNode)
        ctx.console.compileErrorOnToken("Cannot modify immutable variable or parameter", LHSToken);

    if (LHS.isArray() && !assignOperator.isEqualOrLeftArrow())
        ctx.console.compileErrorOnToken("Only '=' or '<-' operator allowed on array", assignOperator);

    if (assignOperator.isBitwiseOperator() && !LHS.isIntegerType())
        ctx.console.compileErrorOnToken("Bitwise operators only allowed on integer types", assignOperator);

    if (assignOperator.getType() == token::EQUAL)
    {
        if (LHS.isUserPointer() && !assignmentNode)
            ctx.console.compileErrorOnToken("Non pointer initialization for pointer", assignOperator);

        if (LHS.isArray() && RHS.isUserPointer())
            ctx.console.compileErrorOnToken("Cannot assign pointer to array", RHSToken);

        ctx.ir.functionBuilder.unitCopy(LHS, RHS);
    }
    else if (assignOperator.getType() == token::LEFT_ARROW)
    {
        if (!LHS.isUserPointer())
            ctx.console.compileErrorOnToken("Pointer assignment on a non pointer", assignOperator);

        if (!RHS.isValidForPointerAssignment())
            ctx.console.compileErrorOnToken("Invalid expression for pointer assignment", RHSToken);

        if (!RHS.isMutable() && RHS.isValue())
            ctx.console.compileErrorOnToken("Cannot assign immutable value to a pointer", RHSToken);

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
    const Token& assignmentOperator = root.getNthChildTokenFromLast(2);
    Unit LHS = ordinaryExpression(ctx, root.children[0]);
    Unit RHS = expression(ctx, root.children[2]);

    assignmentFromTree(ctx, root.type, assignmentOperator, LHSToken, RHSToken, LHS, RHS);
}

bool allIdentifierNodes(const Node& LHSNode)
{
    for (const Node& child : LHSNode.children)
    {
        if (child.type != node::EXPRESSION)
            return false;

        if (child.children[0].type != node::TERM)
            return false;

        if (child.children[0].children[0].type != node::IDENTIFIER)
            return false;
    }

    return true;
}

std::vector<Token> getNameTokens(const Node& LHSNode)
{
    std::vector<Token> nameTokens;

    for (const Node& child : LHSNode.children)
        nameTokens.push_back(child.children[0].children[0].tok);

    return nameTokens;
}

void orderedDestructuredAssignment(generator::GeneratorContext& ctx, const Node& root, const Unit& LHS, const Unit& RHS)
{
    const Token& assignmentOperator = root.getNthChildTokenFromLast(2);

    std::vector<Unit> destructuredLHS = ctx.ir.functionBuilder.destructureUnit(LHS);
    std::vector<Unit> destructuredRHS = ctx.ir.functionBuilder.destructureUnit(RHS);

    if (destructuredLHS.size() != destructuredRHS.size())
        ctx.console.compileErrorOnToken("Number of assignments do not match", root.tok);

    for (size_t i = 0; i < destructuredLHS.size(); i += 1)
    {
        const Token& LHSToken = root.children[0].children[i].tok;

        assignmentFromTree(ctx,
                           root.type,
                           assignmentOperator,
                           LHSToken,
                           LHSToken,
                           destructuredLHS[i],
                           destructuredRHS[i]);
    }
}

void namedDestructuredAssignment(generator::GeneratorContext& ctx,
                                 const Node& root,
                                 const std::vector<Token> nameTokens,
                                 const Unit& LHS,
                                 const Unit& RHS)
{
    const Token& assignmentOperator = root.getNthChildTokenFromLast(2);

    std::vector<Unit> destructuredLHS = ctx.ir.functionBuilder.destructureUnit(LHS);
    std::map<std::string, Unit> destructuredRHSMapped = ctx.ir.functionBuilder.destructureStructMapped(RHS);

    for (size_t i = 0; i < destructuredLHS.size(); i += 1)
    {
        const Token& LHSToken = root.children[0].children[i].tok;

        assignmentFromTree(ctx,
                           root.type,
                           assignmentOperator,
                           LHSToken,
                           LHSToken,
                           destructuredLHS[i],
                           destructuredRHSMapped.at(nameTokens[i].toString()));
    }
}

void destructuredAssignment(generator::GeneratorContext& ctx, const Node& root)
{
    const Node& LHSNode = root.children[0];
    Unit LHS = expression(ctx, root.children[0]);
    Unit RHS = expression(ctx, root.children[2]);

    if (!RHS.isStructOrArray())
        ctx.console.compileErrorOnToken("Cannot destructure non struct or non array", root.children[2].tok);

    if (allIdentifierNodes(LHSNode) && RHS.isStruct())
    {
        std::vector<Token> nameTokens = getNameTokens(LHSNode);

        if (ctx.ir.descriptionFinder.isAllNamesStructFields(nameTokens, RHS))
            namedDestructuredAssignment(ctx, root, nameTokens, LHS, RHS);
        else
            orderedDestructuredAssignment(ctx, root, LHS, RHS);
    }
    else
        orderedDestructuredAssignment(ctx, root, LHS, RHS);
}
