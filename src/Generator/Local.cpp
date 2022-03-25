#include "../Builder/TypeCheck.hpp"
#include "Assignment.hpp"
#include "Expression.hpp"
#include "TypeDescriptionFromNode.hpp"

#include "Local.hpp"

void local(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    ctx.scope.putInCurrentScope(nameToken);

    icode::TypeDescription localType = typeDescriptionFromNode(ctx, root);

    if (root.type == node::VAR)
        localType.becomeMutable();

    Unit local = ctx.ir.functionBuilder.createLocal(nameToken, localType);

    Node lastNode = root.children.back();

    if (lastNode.type == node::EXPRESSION || lastNode.type == node::TERM ||
        lastNode.type == node::MULTILINE_STR_LITERAL || lastNode.type == node::INITLIST)
    {
        Unit RHS = expression(ctx, lastNode);
        assignmentFromTree(ctx, root, nameToken, lastNode.tok, local, RHS);
    }
}

void createWalrusLocal(generator::GeneratorContext& ctx, const Node& root, const Token& nameToken, const Unit& RHS)
{
    ctx.scope.putInCurrentScope(nameToken);

    icode::TypeDescription localType = RHS.type();
    localType.becomeNonPointer();

    if (root.type == node::WALRUS_VAR || root.type == node::DESTRUCTURE_VAR)
        localType.becomeMutable();
    else
        localType.becomeImmutable();

    localType.decayAutoType();

    Unit local = ctx.ir.functionBuilder.createLocal(nameToken, localType);

    ctx.ir.functionBuilder.unitCopy(local, RHS);
}

void walrusLocal(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);
    const Unit& RHS = expression(ctx, root.children[1]);

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
    std::vector<Token> nameTokens;

    for (const Node& child : root.children[0].children)
        nameTokens.push_back(child.tok);

    Unit RHS = expression(ctx, root.children.back());

    if (!RHS.isStructOrArray())
        ctx.console.compileErrorOnToken("Cannot destructure NON STRUCT or NON ARRAY", root.children.back().tok);

    if (RHS.isStruct() && ctx.ir.descriptionFinder.isAllNamesStructFields(nameTokens, RHS))
        namedDestructure(ctx, root, nameTokens, RHS);
    else
        orderedDestructure(ctx, root, nameTokens, RHS);
}
