#include "DescriptionGenerator/CreateStructDescription.hpp"
#include "VariableDescriptionFromNode.hpp"

#include "Structure.hpp"

void structFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    const token::Token& nameToken = root.getNthChildToken(0);

    std::vector<token::Token> fieldNames;
    std::vector<icode::VariableDescription> fieldTypes;
    for (size_t i = 0; i < root.children[0].children.size(); i++)
    {
        TokenDescriptionPair tokenDescriptionPair = variableDescriptionFromNode(ctx, root.children[0].children[i]);
        tokenDescriptionPair.second.setProperty(icode::IS_MUT);

        fieldNames.push_back(tokenDescriptionPair.first);
        fieldTypes.push_back(tokenDescriptionPair.second);
    }

    createStructDescription(ctx, nameToken, fieldNames, fieldTypes);
}