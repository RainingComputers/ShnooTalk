#include "ConditionalExpression.hpp"
#include "OrdinaryExpression.hpp"

#include "Expression.hpp"

Unit expression(generator::GeneratorContext& ctx, const Node& root)
{
    if (!root.isConditionalExpression())
        return ordinaryExpression(ctx, root);

    Unit LHS = ctx.ir.functionBuilder.createTemp(icode::AUTO_INT);

    icode::Operand trueLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "condn_exp");
    icode::Operand falseLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "condn_exp");

    icode::Operand endLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "condn_exp_end");

    conditionalExpression(ctx, root, trueLabel, falseLabel, true);

    ctx.ir.functionBuilder.unitCopy(LHS, ctx.ir.unitBuilder.unitFromIntLiteral(1));
    ctx.ir.functionBuilder.createBranch(icode::GOTO, endLabel);

    ctx.ir.functionBuilder.insertLabel(falseLabel);

    ctx.ir.functionBuilder.unitCopy(LHS, ctx.ir.unitBuilder.unitFromIntLiteral(0));

    ctx.ir.functionBuilder.insertLabel(endLabel);

    return LHS;
}
