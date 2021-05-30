#include "TypeDescriptionFromNode.hpp"

#include "Structure.hpp"

using namespace icode;

void createStructFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    const token::Token& nameToken = root.getNthChildToken(0);

    std::vector<token::Token> fieldNames;
    std::vector<icode::TypeDescription> fieldTypes;

    for (size_t i = 0; i < root.children[0].children.size(); i++)
    {
        const token::Token& fieldName = root.children[0].children[i].getNthChildToken(0);

        TypeDescription fieldType = typeDescriptionFromNode(ctx, root.children[0].children[i]);
        fieldType.setProperty(IS_MUT);

        fieldNames.push_back(fieldName);
        fieldTypes.push_back(fieldType);
    }

    ctx.descriptionBuilder.createStructDescription(nameToken, fieldNames, fieldTypes);
}