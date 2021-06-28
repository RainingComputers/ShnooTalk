#include "Expression.hpp"

#include "UnitFromIdentifier.hpp"

Unit getUnitFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    Unit Unit = ctx.descriptionFinder.getUnitFromToken(nameToken);

    if (Unit.type.checkProperty(icode::IS_ENUM) && root.children.size() > 1)
        ctx.console.compileErrorOnToken("Invalid use of ENUM", nameToken);

    if (Unit.type.checkProperty(icode::IS_DEFINE) && root.children.size() > 1)
        ctx.console.compileErrorOnToken("Invalid use of DEF", nameToken);

    if (Unit.type.checkProperty(icode::IS_LOCAL))
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

    if (unit.type.dtype != icode::STRUCT)
        ctx.console.compileErrorOnToken("STRUCT access on a NON-STRUCT data type", fieldNameToken);

    if (unit.type.dimensions.size() != 0)
        ctx.console.compileErrorOnToken("STRUCT access on an ARRAY", fieldNameToken);

    nodeCounter++;

    const Unit returnUnit = ctx.functionBuilder.getStructField(fieldNameToken, unit);

    return std::pair<Unit, size_t>(returnUnit, nodeCounter);
}

std::pair<Unit, size_t> unitFromExpressionSubscripts(generator::GeneratorContext& ctx,
                                                     const Node& root,
                                                     const Unit& unit,
                                                     size_t startIndex)
{
    size_t nodeCounter = startIndex;

    if (unit.type.dimensions.size() == 0)
        ctx.console.compileErrorOnToken("ARRAY access on a NON ARRAY", root.children[nodeCounter].tok);

    std::vector<Unit> indices;

    for (; nodeCounter < root.children.size(); nodeCounter++)
    {
        const Node& child = root.children[nodeCounter];

        if (!child.isNodeType(node::SUBSCRIPT))
            break;

        const Unit indexExpression = expression(ctx, child.children[0]);

        indices.push_back(indexExpression);

        if (indices.size() > unit.type.dimensions.size())
            ctx.console.compileErrorOnToken("Too many subscripts", child.tok);

        if (!icode::isInteger(indexExpression.type.dtype) || indexExpression.type.isArray())
            ctx.console.compileErrorOnToken("Index must be an integer", child.children[0].tok);
    }

    const Unit returnUnit = ctx.functionBuilder.getIndexedElement(unit, indices);

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
            case node::STRUCT_VAR:
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