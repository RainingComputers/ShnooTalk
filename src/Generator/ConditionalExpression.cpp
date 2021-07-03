#include "Expression.hpp"

#include "ConditionalExpression.hpp"

icode::Instruction tokenToCompareOperator(const generator::GeneratorContext& ctx, const Token tok)
{
    switch (tok.getType())
    {
        case token::LESS_THAN:
            return icode::LT;
        case token::LESS_THAN_EQUAL:
            return icode::LTE;
        case token::GREATER_THAN:
            return icode::GT;
        case token::GREATER_THAN_EQUAL:
            return icode::GTE;
        case token::CONDN_EQUAL:
            return icode::EQ;
        case token::CONDN_NOT_EQUAL:
            return icode::NEQ;
        default:
            ctx.console.compileErrorOnToken("Invalid conditional expression", tok);
    }
}

void conditionalTerm(generator::GeneratorContext& ctx,
                     const Node& root,
                     const icode::Operand& trueLabel,
                     const icode::Operand& falseLabel,
                     bool trueFall)
{
    if (root.children[0].type == node::EXPRESSION)
    {
        conditionalExpression(ctx, root.children[0], trueLabel, falseLabel, trueFall);
        return;
    }

    if (root.children[0].tok.getType() != token::CONDN_NOT)
        ctx.console.compileErrorOnToken("Invalid conditional expression", root.tok);

    if (!root.children[0].isNthChild(node::TERM, 0))
        ctx.console.compileErrorOnToken("Invalid conditional expression", root.tok);

    if (!root.children[0].children[0].isNthChild(node::EXPRESSION, 0))
        ctx.console.compileErrorOnToken("Invalid conditional expression", root.tok);

    conditionalExpression(ctx, root.children[0].children[0].children[0], falseLabel, trueLabel, !trueFall);
}

void conditionalAndOperator(generator::GeneratorContext& ctx,
                            const Token& operatorToken,
                            const Node& root,
                            const icode::Operand& trueLabel,
                            const icode::Operand& falseLabel,
                            bool trueFall)
{
    icode::Operand newTrueLabel = ctx.functionBuilder.createLabel(operatorToken, true, "and");
    icode::Operand newFalseLabel = ctx.functionBuilder.createLabel(operatorToken, false, "and");

    if (trueFall)
        conditionalExpression(ctx, root.children[0], newTrueLabel, falseLabel, true);
    else
        conditionalExpression(ctx, root.children[0], newTrueLabel, newFalseLabel, true);

    conditionalExpression(ctx, root.children[2], trueLabel, falseLabel, trueFall);

    if (!trueFall)
        ctx.functionBuilder.insertLabel(newFalseLabel);
}

void conditionalOrOperator(generator::GeneratorContext& ctx,
                           const Token& operatorToken,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall)
{
    icode::Operand newTrueLabel = ctx.functionBuilder.createLabel(operatorToken, true, "or");
    icode::Operand newFalseLabel = ctx.functionBuilder.createLabel(operatorToken, false, "or");

    if (!trueFall)
        conditionalExpression(ctx, root.children[0], trueLabel, newFalseLabel, false);
    else
        conditionalExpression(ctx, root.children[0], newTrueLabel, newFalseLabel, false);

    conditionalExpression(ctx, root.children[2], trueLabel, falseLabel, trueFall);

    if (trueFall)
        ctx.functionBuilder.insertLabel(newTrueLabel);
}

void relationalOperator(generator::GeneratorContext& ctx,
                        const Token& operatorToken,
                        const Node& root,
                        const icode::Operand& trueLabel,
                        const icode::Operand& falseLabel,
                        bool trueFall)
{
    icode::Instruction opcode = tokenToCompareOperator(ctx, operatorToken);

    Unit LHS = expression(ctx, root.children[0]);
    Unit RHS = expression(ctx, root.children[2]);

    if (LHS.type.isStruct() || LHS.type.isArray())
        ctx.console.compileErrorOnToken("Cannot compare STRUCT or ARRAYS", operatorToken);

    if (!ctx.typeChecker.check(LHS, RHS))
        ctx.console.typeError(root.children[2].tok, LHS.type, RHS.type);

    ctx.functionBuilder.compareOperator(opcode, LHS, RHS);

    if (!trueFall)
        ctx.functionBuilder.createBranch(icode::IF_TRUE_GOTO, trueLabel);
    else
        ctx.functionBuilder.createBranch(icode::IF_FALSE_GOTO, falseLabel);
}

void conditionalExpression(generator::GeneratorContext& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall)
{
    if (root.type == node::TERM)
        conditionalTerm(ctx, root, trueLabel, falseLabel, trueFall);
    else if (root.children.size() == 1)
        conditionalExpression(ctx, root.children[0], trueLabel, falseLabel, trueFall);
    else
    {
        Token operatorToken = root.children[1].tok;

        switch (operatorToken.getType())
        {
            case token::CONDN_AND:
                conditionalAndOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
                break;
            case token::CONDN_OR:
                conditionalOrOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
                break;
            default:
                relationalOperator(ctx, operatorToken, root, trueLabel, falseLabel, trueFall);
        }
    }
}