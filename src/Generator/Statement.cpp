#include "Assignment.hpp"
#include "FunctionCall.hpp"
#include "ConditionalExpression.hpp"
#include "Module.hpp"
#include "Assignment.hpp"
#include "FunctionCall.hpp"
#include "Input.hpp"
#include "Local.hpp"
#include "Module.hpp"
#include "Print.hpp"

#include "Statement.hpp"

using namespace icode;

void ifStatement(generator::GeneratorContext& ctx,
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

            block(ctx, child.children[1], isLoopBlock, loopLabel, breakLabel, continueLabel);

            if (i != root.children.size() - 1)
                ctx.functionBuilder.createBranch(GOTO, elseIfChainEndLabel);

            ctx.functionBuilder.insertLabel(ifFalseLabel);
        }
        else
        {
            block(ctx, child.children[0], isLoopBlock, loopLabel, breakLabel, continueLabel);
        }
    }

    if (root.children.size() != 1)
        ctx.functionBuilder.insertLabel(elseIfChainEndLabel);
}

void whileLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.functionBuilder.createLabel(root.tok, true, "while");
    Operand breakLabel = ctx.functionBuilder.createLabel(root.tok, false, "while");

    ctx.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[0], loopLabel, breakLabel, true);

    block(ctx, root.children[1], true, loopLabel, breakLabel, loopLabel);

    ctx.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.functionBuilder.insertLabel(breakLabel);
}

void forLoop(generator::GeneratorContext& ctx, const Node& root)
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

    block(ctx, root.children[3], true, loopLabel, breakLabel, continueLabel);

    ctx.functionBuilder.insertLabel(continueLabel);

    assignment(ctx, root.children[2]);

    ctx.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.functionBuilder.insertLabel(breakLabel);
}

void continueStatement(generator::GeneratorContext& ctx, bool isLoopBlock, const Operand& continueLabel, const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("CONTINUE outside loop", token);

    ctx.functionBuilder.createBranch(GOTO, continueLabel);
}

void breakStatement(generator::GeneratorContext& ctx, bool isLoopBlock, const Operand& breakLabel, const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("BREAK outside loop", token);

    ctx.functionBuilder.createBranch(GOTO, breakLabel);
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
        case node::ASSIGNMENT:
        case node::ASSIGNMENT_STR:
        case node::ASSIGNMENT_INITLIST:
            assignment(ctx, root);
            break;
        case node::FUNCCALL:
            functionCall(ctx, root);
            break;
        case node::TERM:
            functionCall(ctx, root.children[0]);
            break;
        case node::IF:
            ifStatement(ctx, root, isLoopBlock, loopLabel, breakLabel, continueLabel);
            break;
        case node::WHILE:
            whileLoop(ctx, root);
            break;
        case node::FOR:
            forLoop(ctx, root);
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
        case node::EXIT:
            ctx.functionBuilder.noArgumentEntry(EXIT);
            break;
        case node::MODULE:
        {
            ctx.pushWorkingModule();
            int nodeCounter = setWorkingModuleFromNode(ctx, root, 0);
            functionCall(ctx, root.children[nodeCounter]);
            ctx.popWorkingModule();
            break;
        }
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

    for (Node stmt : root.children)
        statement(ctx, stmt, isLoopBlock, loopLabel, breakLabel, continueLabel);

    ctx.scope.exitScope();
}
