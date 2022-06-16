#include "../Builder/TypeDescriptionUtil.hpp"
#include "Generic.hpp"
#include "Module.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace icode;

TypeDescription arrayTypeFromSubscript(const Node& root, const TypeDescription& typeDescription, size_t startIndex)
{
    std::vector<int> dimensions;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::SUBSCRIPT, nodeCounter); nodeCounter++)
    {
        const int subscriptInt = root.children[nodeCounter].children[0].tok.toInt();
        dimensions.push_back(subscriptInt);
    }

    return createArrayTypeDescription(typeDescription, dimensions, FIXED_DIM);
}

TypeDescription addTypeModifiers(generator::GeneratorContext& ctx,
                                 const Node& root,
                                 const TypeDescription& typeDescription,
                                 size_t startIndex,
                                 const Token& token)
{
    size_t nodeCounter = startIndex;

    TypeDescription modifiedTypeDescription = typeDescription;

    if (root.isNthChild(node::POINTER_STAR, nodeCounter))
        modifiedTypeDescription.becomePointer();

    if (root.isNthChild(node::EMPTY_SUBSCRIPT, nodeCounter))
        modifiedTypeDescription.becomeArrayPointer();

    if (root.isNthChild(node::SUBSCRIPT, nodeCounter))
        modifiedTypeDescription = arrayTypeFromSubscript(root, typeDescription, nodeCounter);

    if (modifiedTypeDescription.isIncompleteType() && !modifiedTypeDescription.isPointer())
        ctx.console.compileErrorOnToken("Incomplete type can only be a pointer", token);

    return modifiedTypeDescription;
}

TypeDescription getMonomorphizedTypeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();
    ctx.ir.resetWorkingModule();
    ctx.mm.resetWorkingModule();

    size_t childNodeCounter = 1;
    if (root.type == node::GENERIC_TYPE_PARAM || root.type == node::MAKE || root.type == node::SIZEOF)
        childNodeCounter = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::MODULE, childNodeCounter))
    {
        const Token aliasToken = root.getNthChildToken(childNodeCounter);

        if (childNodeCounter > 1)
            ctx.console.compileErrorOnToken("Invalid namespace access from generic", aliasToken);

        ctx.mm.setWorkingModuleFromAlias(aliasToken);

        childNodeCounter++;
    }

    const Token genericStructNameToken = root.getNthChildToken(childNodeCounter);
    childNodeCounter++;

    const std::string genericModuleName = ctx.mm.getGenericModuleFromToken(genericStructNameToken);

    std::vector<TypeDescription> instantiationTypes;
    std::vector<Node> instantiationTypeNodes;

    while (root.isNthChild(node::GENERIC_TYPE_PARAM, childNodeCounter))
    {
        instantiationTypes.push_back(typeDescriptionFromNode(ctx, root.children[childNodeCounter]));
        instantiationTypeNodes.push_back(root.children[childNodeCounter]);

        childNodeCounter++;
    }

    TypeDescription monomorphizedType = instantiateGenericAndGetType(ctx,
                                                                     genericModuleName,
                                                                     genericStructNameToken,
                                                                     instantiationTypes,
                                                                     instantiationTypeNodes);

    monomorphizedType = addTypeModifiers(ctx, root, monomorphizedType, childNodeCounter, genericStructNameToken);

    ctx.ir.popWorkingModule();

    return monomorphizedType;
}

TypeDescription typeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();
    ctx.ir.resetWorkingModule();

    if (root.isGenericTypeParamPresent())
        return getMonomorphizedTypeDescriptionFromNode(ctx, root);

    size_t childNodeCounter = 1;
    if (root.type == node::GENERIC_TYPE_PARAM || root.type == node::MAKE || root.type == node::SIZEOF)
        childNodeCounter = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    if (!root.isNthChild(node::IDENTIFIER, childNodeCounter))
        return ctx.ir.moduleBuilder.createVoidTypeDescription();

    const Token dataTypeToken = root.getNthChildToken(childNodeCounter);
    TypeDescription typeDescription = ctx.ir.moduleBuilder.createTypeDescription(dataTypeToken);

    childNodeCounter++;

    typeDescription = addTypeModifiers(ctx, root, typeDescription, childNodeCounter, dataTypeToken);

    ctx.ir.popWorkingModule();

    return typeDescription;
}
