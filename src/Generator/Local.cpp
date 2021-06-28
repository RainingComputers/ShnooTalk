#include "Expression.hpp"
#include "TypeDescriptionFromNode.hpp"

#include "Local.hpp"

void local(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    icode::TypeDescription localType = typeDescriptionFromNode(ctx, root);

    ctx.scope.putInCurrentScope(nameToken);

    if (root.type == node::VAR)
        localType.becomeMutable();

    Unit local = ctx.functionBuilder.createLocal(nameToken, localType);

    Node lastNode = root.children.back();

    if (lastNode.isNodeType(node::EXPRESSION) || lastNode.isNodeType(node::TERM) ||
        lastNode.isNodeType(node::STR_LITERAL) || lastNode.isNodeType(node::INITLIST))
    {
        Unit RHS = expression(ctx, lastNode);

        if (!icode::isSameType(local.type, RHS.type))
            ctx.console.typeError(lastNode.tok, local.type, RHS.type);

        ctx.functionBuilder.unitCopy(local, RHS);
    }
}