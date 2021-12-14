#include "../../TemplateUtils/GetMapElement.hpp"
#include "Instantiator.hpp"
#include "MonomorphNameMangle.hpp"

#include "Monomorphizer.hpp"

using namespace monomorphizer;

Monomorphizer::Monomorphizer(StringGenericASTMap& genericsMap, Console& console)
    : genericsMap(genericsMap)
    , console(console)
{
}

std::vector<std::string> getGenericIdentifiers(const Node& root)
{
    std::vector<std::string> genericIdentifiers;

    for (const Node& child : root.children[0].children)
        genericIdentifiers.push_back(child.tok.toString());

    return genericIdentifiers;
}

bool isGenericIdentifier(const std::string& identifier, const std::vector<std::string>& genericIdentifiers)
{
    auto it = std::find(genericIdentifiers.begin(), genericIdentifiers.end(), identifier);
    return it != genericIdentifiers.end();
}

bool isGenericIdentifierPresent(const Node& root, const std::vector<std::string>& genericIdentifiers)
{
    if (root.type == node::IDENTIFIER)
    {
        if (isGenericIdentifier(root.tok.toString(), genericIdentifiers))
            return true;
    }

    for (const Node& child : root.children)
    {
        if (isGenericIdentifierPresent(child, genericIdentifiers))
            return true;
    }

    return false;
}

std::vector<std::string> Monomorphizer::getGenericStructs(const Node& root,
                                                          const std::vector<std::string>& genericIdentifiers)
{
    std::vector<std::string> genericStructs;

    for (const Node& child : root.children)
    {
        if (child.type != node::STRUCT)
            continue;

        const Token& structNameToken = child.children[0].tok;

        if (isGenericIdentifier(structNameToken.toString(), genericIdentifiers))
            console.compileErrorOnToken("STRUCT name cannot be a GENERIC IDENTIFIER", structNameToken);

        if (isGenericIdentifierPresent(child.children[0], genericIdentifiers))
            genericStructs.push_back(structNameToken.toString());
    }

    return genericStructs;
}

void Monomorphizer::indexAST(const std::string& genericModuleName, const Node& ast)
{
    std::vector<std::string> genericIdentifiers = getGenericIdentifiers(ast);
    std::vector<std::string> genericStructs = getGenericStructs(ast, genericIdentifiers);

    GenericASTIndex index = GenericASTIndex{ ast, genericIdentifiers, genericStructs };

    genericsMap[genericModuleName] = index;
}

void Monomorphizer::createUse(const Token& pathToken, const Token& aliasToken)
{
    const std::string& path = pathToken.toUnescapedString();
    const std::string& alias = aliasToken.toString();

    aliases[alias] = path;
}

std::string Monomorphizer::getGenericModuleNameFromAlias(const Token& aliasToken)
{
    std::string genericModuleName;

    if (getMapElement<std::string, std::string>(aliases, aliasToken.toString(), genericModuleName))
        return genericModuleName;

    console.compileErrorOnToken("Alias does not exist or NOT GENERIC", aliasToken);
}

std::string Monomorphizer::getGenericModuleNameFromStruct(const Token& genericStructNameToken)
{
    std::string genericModuleName;

    if (getMapElement<std::string, std::string>(genericUses, genericStructNameToken.toString(), genericModuleName))
        return genericModuleName;

    console.compileErrorOnToken("GENERIC STRUCT does not exist", genericStructNameToken);
}

Node Monomorphizer::instantiateGeneric(const std::string& genericModuleName,
                                       const Token& typeRootToken,
                                       const std::vector<icode::TypeDescription>& instantiationTypes,
                                       const std::vector<Node>& typeDescriptionNodes)
{
    GenericASTIndex index = genericsMap.at(genericModuleName);

    if (index.genericIdentifiers.size() != instantiationTypes.size())
        console.compileErrorOnToken("Number of type parameters don't match", typeRootToken);

    const std::string& instantiationSuffix = constructInstantiationSuffix(instantiationTypes);

    return instantiateAST(index.ast,
                          index.genericIdentifiers,
                          instantiationTypes,
                          typeDescriptionNodes,
                          instantiationSuffix);
}
