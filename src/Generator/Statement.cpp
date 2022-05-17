#include "Assignment.hpp"
#include "Expression.hpp"
#include "Input.hpp"
#include "Local.hpp"
#include "Module.hpp"
#include "Print.hpp"
#include "Return.hpp"

#include "Statement.hpp"

using namespace icode;

void ifStatement(generator::GeneratorContext& ctx,
                 const Node& root,
                 bool isLoopBlock,
                 const Operand& loopLabel,
                 const Operand& breakLabel,
                 const Operand& continueLabel)
{
    Operand elseIfChainEndLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "ifend");

    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        Operand ifTrueLabel = ctx.ir.functionBuilder.createLabel(child.tok, true, "if");
        Operand ifFalseLabel = ctx.ir.functionBuilder.createLabel(child.tok, false, "if");

        if (child.type != node::ELSE)
        {
            conditionalExpression(ctx, child.children[0], ifTrueLabel, ifFalseLabel, true);

            block(ctx, child.children[1], isLoopBlock, loopLabel, breakLabel, continueLabel);

            if (i != root.children.size() - 1)
                ctx.ir.functionBuilder.createBranch(GOTO, elseIfChainEndLabel);

            ctx.ir.functionBuilder.insertLabel(ifFalseLabel);
        }
        else
        {
            block(ctx, child.children[0], isLoopBlock, loopLabel, breakLabel, continueLabel);
        }
    }

    if (root.children.size() != 1)
        ctx.ir.functionBuilder.insertLabel(elseIfChainEndLabel);
}

void whileLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "while");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "while");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[0], loopLabel, breakLabel, true);

    block(ctx, root.children[1], true, loopLabel, breakLabel, loopLabel);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void doWhileLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "do");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "do");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    block(ctx, root.children[0], true, loopLabel, breakLabel, loopLabel);

    conditionalExpression(ctx, root.children[1], loopLabel, breakLabel, true);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void forLoopInitOrUpdateNode(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.type == node::VAR) // TODO: add walrus operator
        local(ctx, root);
    else if (root.type == node::TERM)
        term(ctx, root);
    else
        assignment(ctx, root);
}

void forLoop(generator::GeneratorContext& ctx, const Node& root)
{

    forLoopInitOrUpdateNode(ctx, root.children[0]);

    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "for");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "for");
    Operand continueLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "for_cont");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[1], loopLabel, breakLabel, true);

    block(ctx, root.children[3], true, loopLabel, breakLabel, continueLabel);

    ctx.ir.functionBuilder.insertLabel(continueLabel);

    forLoopInitOrUpdateNode(ctx, root.children[2]);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void infniteLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "loop");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "loop");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    block(ctx, root.children[0], true, loopLabel, breakLabel, loopLabel);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void continueStatement(generator::GeneratorContext& ctx,
                       bool isLoopBlock,
                       const Operand& continueLabel,
                       const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("continue outside loop", token);

    ctx.ir.functionBuilder.createBranch(GOTO, continueLabel);
}

void breakStatement(generator::GeneratorContext& ctx, bool isLoopBlock, const Operand& breakLabel, const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("break outside loop", token);

    ctx.ir.functionBuilder.createBranch(GOTO, breakLabel);
}

void statement(generator::GeneratorContext& ctx,
               const Node& root,
               bool isLoopBlock,
               const Operand& loopLabel,
               const Operand& breakLabel,
               const Operand& continueLabel)
{
    switch (root.type)
    {
        case node::VAR:
        case node::CONST:
            local(ctx, root);
            break;
        case node::WALRUS_VAR:
        case node::WALRUS_CONST:
            walrusLocal(ctx, root);
            break;
        case node::DESTRUCTURE_VAR:
        case node::DESTRUCTURE_CONST:
            destructureLocal(ctx, root);
            break;
        case node::ASSIGNMENT:
            assignment(ctx, root);
            break;
        case node::DESTRUCTURED_ASSIGNMENT:
            destructuredAssignment(ctx, root);
            break;
        case node::TERM:
            term(ctx, root);
            break;
        case node::IF:
            ifStatement(ctx, root, isLoopBlock, loopLabel, breakLabel, continueLabel);
            break;
        case node::WHILE:
            whileLoop(ctx, root);
            break;
        case node::DO_WHILE:
            doWhileLoop(ctx, root);
            break;
        case node::FOR:
            forLoop(ctx, root);
            break;
        case node::LOOP:
            infniteLoop(ctx, root);
            break;
        case node::BREAK:
            breakStatement(ctx, isLoopBlock, breakLabel, root.tok);
            break;
        case node::CONTINUE:
            continueStatement(ctx, isLoopBlock, continueLabel, root.tok);
            break;
        case node::RETURN:
            functionReturn(ctx, root);
            break;
        case node::PRINT:
        case node::PRINTLN:
            print(ctx, root);
            break;
        case node::INPUT:
            input(ctx, root);
            break;
        default:
            ctx.console.internalBugErrorOnToken(root.tok);
    }
}

void block(generator::GeneratorContext& ctx,
           const Node& root,
           bool isLoopBlock,
           const Operand& loopLabel,
           const Operand& breakLabel,
           const Operand& continueLabel)
{
    ctx.scope.createScope();

    for (const Node& stmt : root.children)
        statement(ctx, stmt, isLoopBlock, loopLabel, breakLabel, continueLabel);

    ctx.scope.exitScope();
}
