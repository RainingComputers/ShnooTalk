#include "Expression.hpp"

#include "Print.hpp"

void print(irgen::ir_generator& ctx, const Node& root)
{
    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        if (child.type == node::STR_LITERAL)
            ctx.functionBuilder.createPrintStringLtrl(ctx.strBuilder.createString(child.tok));
        else
        {
            Unit unit = expression(ctx, child);

            if (unit.second.isStruct() || unit.second.isMultiDimArray())
                ctx.console.compileErrorOnToken("Cannot print STRUCT or multi-dimensional ARRAY", child.tok);

            ctx.functionBuilder.createPrint(unit);
        }

        if (i != root.children.size() - 1)
            ctx.functionBuilder.createPrintSpace();

        if (i == root.children.size() - 1 && root.type == node::PRINTLN)
            ctx.functionBuilder.createPrintNewln();
    }
}