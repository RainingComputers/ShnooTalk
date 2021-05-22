#include "CreateVariableDescription.hpp"
#include "Module.hpp"
#include "Subscript.hpp"

#include "VariableDescriptionFromNode.hpp"

using namespace irgen;
using namespace icode;

bool isVoidFunction(const node::node& root)
{
    if (!root.isNthChildFromLast(node::IDENTIFIER, 2))
        return true;

    return false;
}

TokenDescriptionPair variableDescriptionFromFunctionNode(ir_generator& ctx, const node::node& root)
{
    const token::Token& symbolNameToken = root.children[0].tok;

    if (isVoidFunction(root))
        return TokenDescriptionPair(symbolNameToken, constructVoidVariableDesc(ctx));

    token::Token dataTypeToken = root.children[root.children.size() - 2].tok;

    return TokenDescriptionPair(symbolNameToken, variableDescFromDataTypeToken(ctx, dataTypeToken));
}

TokenDescriptionPair variableDescriptionFromVarOrParamNode(ir_generator& ctx, const node::node& root)
{
    const token::Token& symbolNameToken = root.children[0].tok;

    size_t childNodeCounter = 1;

    if (root.isNthChild(node::MODULE, childNodeCounter))
    {
        ModuleIndexPair moduleIndexPair = getModuleFromNode(ctx, root, childNodeCounter);

        ctx.current_ext_module = moduleIndexPair.first;
        childNodeCounter = moduleIndexPair.second;
    }

    const token::Token& dataTypeToken = root.children[childNodeCounter].tok;
    VariableDescription variableDescription = variableDescFromDataTypeToken(ctx, dataTypeToken);

    childNodeCounter++;
    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
    {
        LiteralDimensionsIndexPair literalDimensionsIndexPair = getLiteralDimensionFromNode(root, childNodeCounter);

        variableDescription = addDimensionToVariableDesc(variableDescription, literalDimensionsIndexPair.first);
        childNodeCounter = literalDimensionsIndexPair.second;
    }

    return TokenDescriptionPair(symbolNameToken, variableDescription);
}

TokenDescriptionPair variableDescriptionFromNode(ir_generator& ctx, const node::node& root)
{
    ctx.resetCurrentExternalModule();

    if (root.type == node::FUNCTION)
        return variableDescriptionFromFunctionNode(ctx, root);

    return variableDescriptionFromVarOrParamNode(ctx, root);
}