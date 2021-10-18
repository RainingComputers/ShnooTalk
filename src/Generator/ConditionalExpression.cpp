#include "../Builder/TypeCheck.hpp"
#include "OperatorTokenToInstruction.hpp"
#include "OrdinaryExpression.hpp"

#include "ConditionalExpression.hpp"

void conditionalAndOperator(generator::GeneratorContext& ctx,
                            const Token& operatorToken,
                            const Node& root,
                            const icode::Operand& trueLabel,
                            const icode::Operand& falseLabel,
                            bool trueFall)
{
    icode::Operand newTrueLabel = ctx.ir.functionBuilder.createLabel(operatorToken, true, "and");
    icode::Operand newFalseLabel = ctx.ir.functionBuilder.createLabel(operatorToken, false, "and");

    if (trueFall)
        conditionalExpression(ctx, root.children[0], newTrueLabel, falseLabel, true);
    else
        conditionalExpression(ctx, root.children[0], newTrueLabel, newFalseLabel, true);

    conditionalExpression(ctx, root.children[2], trueLabel, falseLabel, trueFall);

    if (!trueFall)
        ctx.ir.functionBuilder.insertLabel(newFalseLabel);
}

void conditionalOrOperator(generator::GeneratorContext& ctx,
                           const Token& operatorToken,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall)
{
    icode::Operand newTrueLabel = ctx.ir.functionBuilder.createLabel(operatorToken, true, "or");
    icode::Operand newFalseLabel = ctx.ir.functionBuilder.createLabel(operatorToken, false, "or");

    if (!trueFall)
        conditionalExpression(ctx, root.children[0], trueLabel, newFalseLabel, false);
    else
        conditionalExpression(ctx, root.children[0], newTrueLabel, newFalseLabel, false);

    conditionalExpression(ctx, root.children[2], trueLabel, falseLabel, trueFall);

    if (trueFall)
        ctx.ir.functionBuilder.insertLabel(newTrueLabel);
}

void conditionalIntegerTruthyOperator(generator::GeneratorContext& ctx,
                                      const Node& root,
                                      const icode::Operand& trueLabel,
                                      const icode::Operand& falseLabel,
                                      bool trueFall)
{
    Unit LHS = ordinaryExpression(ctx, root);
    Unit RHS = ctx.ir.unitBuilder.unitFromIntLiteral(0);

    if (LHS.isStruct() || LHS.isArray() || !LHS.isIntegerType())
        ctx.console.compileErrorOnToken("Cannot get truth from expression", root.tok);

    ctx.ir.functionBuilder.compareOperator(icode::GT, LHS, RHS);

    if (!trueFall)
        ctx.ir.functionBuilder.createBranch(icode::IF_TRUE_GOTO, trueLabel);
    else
        ctx.ir.functionBuilder.createBranch(icode::IF_FALSE_GOTO, falseLabel);
}

void relationalOperator(generator::GeneratorContext& ctx,
                        const Token& operatorToken,
                        const Node& root,
                        const icode::Operand& trueLabel,
                        const icode::Operand& falseLabel,
                        bool trueFall)
{
    icode::Instruction opcode = tokenToCompareOperator(ctx, operatorToken);

    Unit LHS = ordinaryExpression(ctx, root.children[0]);
    Unit RHS = ordinaryExpression(ctx, root.children[2]);

    if (LHS.isStruct() || LHS.isArray())
        ctx.console.compileErrorOnToken("Cannot compare STRUCT or ARRAYS", operatorToken);

    if (!isSameType(LHS, RHS))
        ctx.console.typeError(root.children[2].tok, LHS, RHS);

    ctx.ir.functionBuilder.compareOperator(opcode, LHS, RHS);

    if (!trueFall)
        ctx.ir.functionBuilder.createBranch(icode::IF_TRUE_GOTO, trueLabel);
    else
        ctx.ir.functionBuilder.createBranch(icode::IF_FALSE_GOTO, falseLabel);
}

void conditionalTerm(generator::GeneratorContext& ctx,
                     const Node& root,
                     const icode::Operand& trueLabel,
                     const icode::Operand& falseLabel,
                     bool trueFall)
{
    if (root.children[0].tok.getType() == token::CONDN_NOT)
        conditionalExpression(ctx, root.children[0].children[0], falseLabel, trueLabel, !trueFall);
    else if (root.children[0].type == node::EXPRESSION)
        conditionalExpression(ctx, root.children[0], trueLabel, falseLabel, trueFall);
    else
        conditionalIntegerTruthyOperator(ctx, root, trueLabel, falseLabel, trueFall);
}

void conditionalExpression(generator::GeneratorContext& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall)
{
    if (root.type == node::STR_LITERAL || root.type == node::INITLIST || root.type == node::MULTILINE_STR_LITERAL)
        ctx.console.compileErrorOnToken("Invalid conditional expression", root.tok);

    if (root.type == node::TERM)
        conditionalTerm(ctx, root, trueLabel, falseLabel, trueFall);
    else if (root.children.size() == 1)
        conditionalExpression(ctx, root.children[0], trueLabel, falseLabel, trueFall);
    else
    {
        Token operatorToken = root.children[1].tok;

        if (operatorToken.getType() == token::CONDN_AND)
            conditionalAndOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        else if (operatorToken.getType() == token::CONDN_OR)
            conditionalOrOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        else if (operatorToken.isConditionalOperator())
            relationalOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        else
            conditionalIntegerTruthyOperator(ctx, root, trueLabel, falseLabel, trueFall);
    }
}