#include "TypeDescriptionFromNode.hpp"

#include "Structure.hpp"

using namespace icode;

void createStructFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    std::vector<Token> fieldNames;
    std::vector<icode::TypeDescription> fieldTypes;

    for (size_t i = 0; i < root.children[0].children.size(); i++)
    {
        const Token& fieldName = root.children[0].children[i].getNthChildToken(0);

        TypeDescription fieldType = typeDescriptionFromNode(ctx, root.children[0].children[i]);

        fieldNames.push_back(fieldName);
        fieldTypes.push_back(fieldType);
    }

    ctx.ir.moduleBuilder.createStruct(nameToken, fieldNames, fieldTypes);
}

void registerIncompleteTypeFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);
    ctx.ir.moduleBuilder.registerIncompleteType(nameToken);
}