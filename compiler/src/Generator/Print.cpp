#include "Expression.hpp"

#include "Print.hpp"

void print(generator::GeneratorContext& ctx, const Node& root)
{
    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        if (child.type == node::SPACE)
        {
            ctx.ir.functionBuilder.noArgumentEntry(icode::SPACE);
        }
        else
        {
            Unit unit = expression(ctx, child);

            if (unit.isStruct() || unit.isMultiDimArray() || (unit.isArray() && !unit.isChar()))
                ctx.console.compileErrorOnToken("Cannot print string or multi dimensional array or non char array",
                                                child.tok);

            ctx.ir.functionBuilder.createPrint(unit);
        }

        if (i == root.children.size() - 1 && root.type == node::PRINTLN)
            ctx.ir.functionBuilder.noArgumentEntry(icode::NEWLN);
    }
}