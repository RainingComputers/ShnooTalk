#include "Expression.hpp"

#include "Print.hpp"

void print(generator::GeneratorContext& ctx, const Node& root)
{
    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        Unit unit = expression(ctx, child);

        if (unit.isStruct() || unit.isMultiDimArray())
            ctx.console.compileErrorOnToken("Cannot print STRUCT or multi-dimensional ARRAY", child.tok);

        ctx.ir.functionBuilder.createPrint(unit);

        if (i != root.children.size() - 1)
            ctx.ir.functionBuilder.noArgumentEntry(icode::SPACE);

        if (i == root.children.size() - 1 && root.type == node::PRINTLN)
            ctx.ir.functionBuilder.noArgumentEntry(icode::NEWLN);
    }
}