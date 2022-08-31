#include "Expression.hpp"

#include "UnitFromIdentifier.hpp"

Unit getUnitFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token nameToken = root.getNthChildToken(0);

    Unit Unit = ctx.ir.finder.getUnitFromToken(nameToken);

    if (Unit.isLocal())
        if (!ctx.scope.isInCurrentScope(nameToken))
            ctx.console.compileErrorOnToken("Symbol not in scope", nameToken);

    return Unit;
}

std::pair<Unit, size_t> structField(generator::GeneratorContext& ctx,
                                    const Node& root,
                                    const Unit& unit,
                                    size_t startIndex)
{
    size_t nodeCounter = startIndex;

    nodeCounter++;

    const Token fieldNameToken = root.getNthChildToken(nodeCounter);

    if (!unit.isStruct())
        ctx.console.compileErrorOnToken("Cannot get struct field on a non struct data type", fieldNameToken);

    if (unit.isArray())
        ctx.console.compileErrorOnToken("Cannot get struct on an array", fieldNameToken);

    nodeCounter++;

    const Unit returnUnit = ctx.ir.functionBuilder.getStructField(fieldNameToken, unit);

    return std::pair<Unit, size_t>(returnUnit, nodeCounter);
}

std::pair<Unit, size_t> subscript(generator::GeneratorContext& ctx,
                                  const Node& root,
                                  const Unit& unit,
                                  size_t startIndex);

std::pair<Unit, size_t> subscriptOperator(generator::GeneratorContext& ctx,
                                          const Node& root,
                                          const Unit& unit,
                                          size_t startIndex)
{
    size_t nodeCounter = startIndex;

    const Node child = root.children[nodeCounter];

    const Token indexExpressionToken = child.children[0].tok;
    const Unit indexExpression = expression(ctx, child.children[0]);

    const Unit result = createOperatorFunctionCall(ctx,
                                                   "__subscript",
                                                   root.tok,
                                                   indexExpressionToken,
                                                   unit,
                                                   indexExpression,
                                                   child.tok);

    nodeCounter++;

    if (root.isNthChild(node::SUBSCRIPT, nodeCounter))
        return subscript(ctx, root, result, nodeCounter);

    return std::pair<Unit, size_t>(result, nodeCounter);
}

std::pair<Unit, size_t> subscript(generator::GeneratorContext& ctx,
                                  const Node& root,
                                  const Unit& unit,
                                  size_t startIndex)
{
    size_t nodeCounter = startIndex;

    if (unit.isStruct() && !unit.isArray())
        return subscriptOperator(ctx, root, unit, startIndex);

    if (!unit.isArray())
        ctx.console.compileErrorOnToken("Cannot index a non array", root.children[nodeCounter].tok);

    std::vector<Unit> indices;

    for (; nodeCounter < root.children.size(); nodeCounter++)
    {
        const Node child = root.children[nodeCounter];

        if (child.type != node::SUBSCRIPT)
            break;

        const Unit indexExpression = expression(ctx, child.children[0]);

        indices.push_back(indexExpression);

        if (indices.size() > unit.dimensions().size())
            ctx.console.compileErrorOnToken("Too many subscripts", child.tok);

        if (!indexExpression.isIntegerType() || indexExpression.isArray())
            ctx.console.compileErrorOnToken("Index must be an integer", child.children[0].tok);
    }

    const Unit returnUnit = ctx.ir.functionBuilder.getIndexedElement(unit, indices);

    return std::pair<Unit, size_t>(returnUnit, nodeCounter);
}

Unit getUnitFromIdentifier(generator::GeneratorContext& ctx, const Node& root)
{
    Unit unit = getUnitFromNode(ctx, root);

    for (size_t nodeCounter = 1; nodeCounter < root.children.size();)
    {
        std::pair<Unit, size_t> unitAndNodeCounter;

        switch (root.children[nodeCounter].type)
        {
            case node::STRUCT_FIELD:
                unitAndNodeCounter = structField(ctx, root, unit, nodeCounter);
                break;
            case node::SUBSCRIPT:
                unitAndNodeCounter = subscript(ctx, root, unit, nodeCounter);
                break;
            default:
                ctx.console.internalBugErrorOnToken(root.children[nodeCounter].tok);
        }

        unit = unitAndNodeCounter.first;
        nodeCounter = unitAndNodeCounter.second;
    }

    return unit;
}
