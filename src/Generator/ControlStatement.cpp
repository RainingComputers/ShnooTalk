#include "Assignment.hpp"
#include "ConditionalExpression.hpp"
#include "Local.hpp"

#include "ControlStatement.hpp"

using namespace icode;

void ifStatement(irgen::ir_generator& ctx,
                 const Node& root,
                 bool isLoopBlock,
                 const Operand& loopLabel,
                 const Operand& breakLabel,
                 const Operand& continueLabel)
{
    Operand elseIfChainEndLabel = ctx.functionBuilder.createLabel(root.tok, false, "ifend");

    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        Operand ifTrueLabel = ctx.functionBuilder.createLabel(child.tok, true, "if");
        Operand ifFalseLabel = ctx.functionBuilder.createLabel(child.tok, false, "if");

        if (child.type != node::ELSE)
        {
            conditionalExpression(ctx, child.children[0], ifTrueLabel, ifFalseLabel, true);

            ctx.block(child.children[1], isLoopBlock, loopLabel, breakLabel, continueLabel);

            if (i != root.children.size() - 1)
                ctx.functionBuilder.createBranch(GOTO, elseIfChainEndLabel);

            ctx.functionBuilder.insertLabel(ifFalseLabel);
        }
        else
        {
            ctx.block(child.children[0], isLoopBlock, loopLabel, breakLabel, continueLabel);
        }
    }

    if (root.children.size() != 1)
        ctx.functionBuilder.insertLabel(elseIfChainEndLabel);
}

void whileLoop(irgen::ir_generator& ctx, const Node& root)
{
    Operand loopLabel = ctx.functionBuilder.createLabel(root.tok, true, "while");
    Operand breakLabel = ctx.functionBuilder.createLabel(root.tok, false, "while");

    ctx.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[0], loopLabel, breakLabel, true);

    ctx.block(root.children[1], true, loopLabel, breakLabel, loopLabel);

    ctx.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.functionBuilder.insertLabel(breakLabel);
}

void forLoop(irgen::ir_generator& ctx, const Node& root)
{
    if (root.children[0].type == node::VAR)
        local(ctx, root.children[0]);
    else
        assignment(ctx, root.children[0]);

    Operand loopLabel = ctx.functionBuilder.createLabel(root.tok, true, "for");
    Operand breakLabel = ctx.functionBuilder.createLabel(root.tok, false, "for");
    Operand continueLabel = ctx.functionBuilder.createLabel(root.tok, true, "for_cont");

    ctx.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[1], loopLabel, breakLabel, true);

    ctx.block(root.children[3], true, loopLabel, breakLabel, continueLabel);

    ctx.functionBuilder.insertLabel(continueLabel);

    assignment(ctx, root.children[2]);

    ctx.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.functionBuilder.insertLabel(breakLabel);
}

void continueStatement(irgen::ir_generator& ctx, bool isLoopBlock, const Operand& continueLabel, const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("CONTINUE outside loop", token);

    ctx.functionBuilder.createBranch(GOTO, continueLabel);
}

void breakStatement(irgen::ir_generator& ctx, bool isLoopBlock, const Operand& breakLabel, const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("BREAK outside loop", token);

    ctx.functionBuilder.createBranch(GOTO, breakLabel);
}