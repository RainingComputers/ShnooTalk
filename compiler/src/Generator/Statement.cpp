#include <algorithm>

#include "../Builder/TypeDescriptionUtil.hpp"
#include "Assignment.hpp"
#include "Expression.hpp"
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
    Operand elseIfChainEndLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "ifend");

    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        Operand ifTrueLabel = ctx.ir.functionBuilder.createLabel(child.tok, true, "if");
        Operand ifFalseLabel = ctx.ir.functionBuilder.createLabel(child.tok, false, "if");

        if (child.type == node::IF)
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
    if (root.type == node::VAR)
        local(ctx, root);
    else if (root.type == node::WALRUS_VAR)
        walrusLocal(ctx, root);
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

Unit forEachLoopInitCounter(generator::GeneratorContext ctx)
{
    const Unit counter = ctx.ir.functionBuilder.createTemp(icode::AUTO_INT);
    ctx.ir.functionBuilder.unitCopy(counter, ctx.ir.unitBuilder.unitFromIntLiteral(0));

    return counter;
}

std::vector<Unit> forEachLoopInitItemSingle(generator::GeneratorContext& ctx,
                                            const Node& root,
                                            FunctionDescription subscriptFunc)
{
    const Token itemToken = root.getNthChildToken(0);

    TypeDescription itemType = subscriptFunc.functionReturnType;
    itemType.becomeMutable();

    Unit item = createLocal(ctx, itemToken, itemType);

    return { item };
}

std::vector<Unit> forEachLoopInitItemOrderedDestructure(generator::GeneratorContext& ctx,
                                                        const Node& root,
                                                        const std::vector<Token>& nameTokens,
                                                        const TypeDescription& itemType)
{
    std::vector<Unit> items;

    if (itemType.isArray())
    {
        if (nameTokens.size() != itemType.numElements())
            ctx.console.compileErrorOnToken("Number of for each locals do not match", root.tok);

        TypeDescription elementType = getElementType(itemType);

        for (const Token& nameToken : nameTokens)
            items.push_back(createLocal(ctx, nameToken, elementType));
    }
    else
    {
        std::vector<TypeDescription> types = ctx.ir.finder.destructureStructType(itemType);

        if (nameTokens.size() != types.size())
            ctx.console.compileErrorOnToken("Number of for each locals do not match", root.tok);

        for (size_t i = 0; i < nameTokens.size(); i++)
            items.push_back(createLocal(ctx, nameTokens[i], types[i]));
    }

    return items;
}

std::vector<Unit> forEachLoopInitItemNamedDestructure(generator::GeneratorContext& ctx,
                                                      const Node& root,
                                                      const std::vector<Token>& nameTokens,
                                                      const TypeDescription& itemType)
{
    std::map<std::string, TypeDescription> mappedTypes = ctx.ir.finder.destructureStructTypeMapped(itemType);

    std::vector<Unit> items;

    for (const auto& fieldNameAndType : mappedTypes)
    {
        auto result = std::find_if(nameTokens.begin(),
                                   nameTokens.end(),
                                   [fieldNameAndType](Token tok) { return tok.toString() == fieldNameAndType.first; });

        if (result == nameTokens.end())
            ctx.console.internalBugErrorOnToken(root.tok);

        const Token nameToken = *result;
        TypeDescription localType = fieldNameAndType.second;

        items.push_back(createLocal(ctx, nameToken, localType));
    }

    return items;
}

std::vector<Unit> forEachLoopInitItem(generator::GeneratorContext& ctx,
                                      const Node& root,
                                      FunctionDescription subscriptFunc)
{

    if (!root.isNthChild(node::DESTRUCTURE_LIST, 0))
        return forEachLoopInitItemSingle(ctx, root, subscriptFunc);

    if (!subscriptFunc.functionReturnType.isStructOrArrayAndNotPointer())
        ctx.console.compileErrorOnToken("Cannot destructure non struct or non fixed dim array", root.children[0].tok);

    const std::vector<Token> nameTokens = root.children[0].getAllChildTokens();

    const TypeDescription& itemType = subscriptFunc.functionReturnType;

    if (ctx.ir.finder.isAllNamesStructFields(nameTokens, itemType))
        return forEachLoopInitItemNamedDestructure(ctx, root, nameTokens, itemType);
    else
        return forEachLoopInitItemOrderedDestructure(ctx, root, nameTokens, itemType);
}

void forEachLoopNextItem(generator::GeneratorContext& ctx,
                         const Node& root,
                         const std::vector<Unit>& items,
                         const Unit& nextItem)
{
    if (root.isNthChild(node::DESTRUCTURE_LIST, 0))
    {
        const std::vector<Unit> destructuredNextItems = ctx.ir.functionBuilder.destructureUnit(nextItem);

        if (items.size() != destructuredNextItems.size())
            ctx.console.internalBugErrorOnToken(root.tok);

        for (size_t i = 0; i < items.size(); i += 1)
            ctx.ir.functionBuilder.unitCopy(items[i], destructuredNextItems[i]);
    }
    else
    {
        ctx.ir.functionBuilder.unitCopy(items[0], nextItem);
    }
}

void forEachLoop(generator::GeneratorContext& ctx, const Node& root)
{
    const Token expressionToken = root.getNthChildToken(1);
    const Token itemToken = root.getNthChildToken(0);

    Unit generator = expression(ctx, root.children[1]);

    /* Get generator methods */
    FunctionDescription lenFunc = ctx.ir.finder.getMethodFromUnit(generator, "length", expressionToken);
    FunctionDescription subscriptFunc =
        ctx.ir.finder.getMethodFromUnit(generator, "__subscriptItem__", expressionToken);

    /* Declare for each element as local  */
    std::vector<Unit> item = forEachLoopInitItem(ctx, root, subscriptFunc);

    /* Create a temp variable for counter */
    Unit counter = forEachLoopInitCounter(ctx);

    /* Create loop labels */
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "for_each");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "for_each");
    Operand continueLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "for_each_cont");

    /* Label for beginning of loop */
    ctx.ir.functionBuilder.insertLabel(loopLabel);

    /* Loop condition counter < generator.length() */
    const Unit len = createCallFunction(ctx, { expressionToken }, { generator }, lenFunc, expressionToken);
    ctx.ir.functionBuilder.compareOperator(icode::LT, counter, len);
    createJumps(ctx, loopLabel, breakLabel, true);

    /* Loop item x = generator.__subscriptItem__(counter) */
    const Unit nextItem =
        createCallFunction(ctx, { expressionToken, itemToken }, { generator, counter }, subscriptFunc, expressionToken);
    forEachLoopNextItem(ctx, root, item, nextItem);

    /* For loop block */
    block(ctx, root.children[2], true, loopLabel, breakLabel, continueLabel);

    ctx.ir.functionBuilder.insertLabel(continueLabel);

    /* Loop update counter += 1 */
    const Unit counterPlusOne =
        ctx.ir.functionBuilder.binaryOperator(icode::ADD, counter, ctx.ir.unitBuilder.unitFromIntLiteral(1));
    ctx.ir.functionBuilder.unitCopy(counter, counterPlusOne);

    /* Jump to beginning of loop */
    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    /* Label to exit the loop */
    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void infiniteLoop(generator::GeneratorContext& ctx, const Node& root)
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
        case node::FOR_EACH:
            forEachLoop(ctx, root);
            break;
        case node::LOOP:
            infiniteLoop(ctx, root);
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
