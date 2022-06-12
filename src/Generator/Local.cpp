#include "../Builder/TypeCheck.hpp"
#include "Assignment.hpp"
#include "Expression.hpp"
#include "TypeDescriptionFromNode.hpp"

#include "Local.hpp"

Unit createLocal(generator::GeneratorContext& ctx, const Token& nameToken, icode::TypeDescription& type)
{
    ctx.scope.putInCurrentScope(nameToken);
    return ctx.ir.functionBuilder.createLocal(nameToken, type);
}

void local(generator::GeneratorContext& ctx, const Node& root)
{
    const Token nameToken = root.getNthChildToken(0);

    icode::TypeDescription localType = typeDescriptionFromNode(ctx, root);

    if (root.type == node::VAR)
        localType.becomeMutable();

    Unit local = createLocal(ctx, nameToken, localType);

    Node lastNode = root.children.back();

    if (lastNode.type == node::EXPRESSION || lastNode.type == node::TERM ||
        lastNode.type == node::MULTILINE_STR_LITERAL || lastNode.type == node::INITLIST)
    {
        Unit RHS = expression(ctx, lastNode);
        assignmentFromTree(ctx, root.type, root.getNthChildTokenFromLast(2), nameToken, lastNode.tok, local, RHS);
    }
}

void createWalrusLocal(generator::GeneratorContext& ctx, const Node& root, const Token& nameToken, const Unit& RHS)
{

    icode::TypeDescription localType = RHS.type();
    localType.becomeNonPointer();

    if (localType.isVoid())
        ctx.console.compileErrorOnToken("Cannot assign a void type", root.children.back().tok);

    if (root.type == node::WALRUS_VAR || root.type == node::DESTRUCTURE_VAR)
        localType.becomeMutable();
    else
        localType.becomeImmutable();

    localType.decayAutoType();

    Unit local = createLocal(ctx, nameToken, localType);

    ctx.ir.functionBuilder.unitCopy(local, RHS);
}

void walrusLocal(generator::GeneratorContext& ctx, const Node& root)
{
    const Token nameToken = root.getNthChildToken(0);
    const Unit RHS = expression(ctx, root.children[1]);

    createWalrusLocal(ctx, root, nameToken, RHS);
}

void orderedDestructure(generator::GeneratorContext& ctx,
                        const Node& root,
                        const std::vector<Token>& nameTokens,
                        const Unit& RHS)
{
    const std::vector<Unit> destructuredUnits = ctx.ir.functionBuilder.destructureUnit(RHS);

    if (nameTokens.size() != destructuredUnits.size())
        ctx.console.compileErrorOnToken("Number of locals do not match", root.tok);

    for (size_t i = 0; i < nameTokens.size(); i += 1)
        createWalrusLocal(ctx, root, nameTokens[i], destructuredUnits[i]);
}

void namedDestructure(generator::GeneratorContext& ctx,
                      const Node& root,
                      const std::vector<Token>& nameTokens,
                      const Unit& RHS)
{
    const std::map<std::string, Unit> mappedDestructuredUnits = ctx.ir.functionBuilder.destructureStructMapped(RHS);

    for (size_t i = 0; i < nameTokens.size(); i += 1)
        createWalrusLocal(ctx, root, nameTokens[i], mappedDestructuredUnits.at(nameTokens[i].toString()));
}

void destructureLocal(generator::GeneratorContext& ctx, const Node& root)
{
    const std::vector<Token> nameTokens = root.children[0].getAllChildTokens();

    Unit RHS = expression(ctx, root.children.back());

    if (!RHS.isStructOrArrayAndNotPointer())
        ctx.console.compileErrorOnToken("Cannot destructure non struct or non fixed dim array",
                                        root.children.back().tok);

    if (ctx.ir.finder.isAllNamesStructFields(nameTokens, RHS.type()))
        namedDestructure(ctx, root, nameTokens, RHS);
    else
        orderedDestructure(ctx, root, nameTokens, RHS);
}
