#include "Expression.hpp"

#include "UnitFromIdentifier.hpp"

Unit getUnitFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    Unit Unit = ctx.ir.descriptionFinder.getUnitFromToken(nameToken);

    if (Unit.isLocal())
        if (!ctx.scope.isInCurrentScope(nameToken))
            ctx.console.compileErrorOnToken("Symbol not in scope", nameToken);

    return Unit;
}

std::pair<Unit, size_t> unitFromStructVar(generator::GeneratorContext& ctx,
                                          const Node& root,
                                          const Unit& unit,
                                          size_t startIndex)
{
    size_t nodeCounter = startIndex;

    nodeCounter++;

    const Token& fieldNameToken = root.getNthChildToken(nodeCounter);

    if (!unit.isStruct())
        ctx.console.compileErrorOnToken("STRUCT access on a NON-STRUCT data type", fieldNameToken);

    if (unit.isArray())
        ctx.console.compileErrorOnToken("STRUCT access on an ARRAY", fieldNameToken);

    nodeCounter++;

    const Unit returnUnit = ctx.ir.functionBuilder.getStructField(fieldNameToken, unit);

    return std::pair<Unit, size_t>(returnUnit, nodeCounter);
}

std::pair<Unit, size_t> unitFromExpressionSubscripts(generator::GeneratorContext& ctx,
                                                     const Node& root,
                                                     const Unit& unit,
                                                     size_t startIndex)
{
    size_t nodeCounter = startIndex;

    if (!unit.isArray())
        ctx.console.compileErrorOnToken("ARRAY access on a NON ARRAY", root.children[nodeCounter].tok);

    std::vector<Unit> indices;

    for (; nodeCounter < root.children.size(); nodeCounter++)
    {
        const Node& child = root.children[nodeCounter];

        if (!(child.type == node::SUBSCRIPT))
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
                unitAndNodeCounter = unitFromStructVar(ctx, root, unit, nodeCounter);
                break;
            case node::SUBSCRIPT:
                unitAndNodeCounter = unitFromExpressionSubscripts(ctx, root, unit, nodeCounter);
                break;
            default:
                ctx.console.internalBugErrorOnToken(root.children[nodeCounter].tok);
        }

        unit = unitAndNodeCounter.first;
        nodeCounter = unitAndNodeCounter.second;
    }

    return unit;
}