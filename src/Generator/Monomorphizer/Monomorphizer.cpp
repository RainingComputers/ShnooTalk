#include <algorithm>

#include "../../TemplateUtils/GetMapElement.hpp"
#include "Instantiator.hpp"

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

bool elementInList(const std::string& identifier, const std::vector<std::string>& genericIdentifiers)
{
    auto it = std::find(genericIdentifiers.begin(), genericIdentifiers.end(), identifier);
    return it != genericIdentifiers.end();
}

bool genericTypePresent(const Node& root,
                        const std::vector<std::string>& genericIdentifiers,
                        const std::vector<std::string> genericStructs)
{
    if (root.type == node::IDENTIFIER)
    {
        if (elementInList(root.tok.toString(), genericIdentifiers))
            return true;

        if (elementInList(root.tok.toString(), genericStructs))
            return true;
    }

    for (const Node& child : root.children)
    {
        if (genericTypePresent(child, genericIdentifiers, genericStructs))
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

        if (elementInList(structNameToken.toString(), genericIdentifiers))
            console.compileErrorOnToken("STRUCT name cannot be a GENERIC IDENTIFIER", structNameToken);

        if (genericTypePresent(child.children[0], genericIdentifiers, genericStructs))
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
    uses.push_back(path);
}

bool Monomorphizer::useExists(const Token& pathToken)
{
    const std::string& path = pathToken.toUnescapedString();
    return std::find(uses.begin(), uses.end(), path) != uses.end();
}

bool Monomorphizer::aliasExists(const Token& aliasToken)
{
    return aliases.find(aliasToken.toString()) != aliases.end();
}

void Monomorphizer::createFrom(const std::string& genericModuleName, const Token& symbolToken)
{
    const std::string& structName = symbolToken.toString();

    GenericASTIndex& index = genericsMap.at(genericModuleName);
    const std::vector<std::string>& genericStructs = index.genericStructs;

    if (std::find(genericStructs.begin(), genericStructs.end(), structName) == genericStructs.end())
        console.compileErrorOnToken("GENERIC does not exist", symbolToken);

    genericUses[structName] = genericModuleName;
}

void Monomorphizer::createDirectFrom(const Token& pathToken, const Token& symbolToken)
{
    const std::string& path = pathToken.toUnescapedString();

    if (!useExists(path))
        uses.push_back(path);

    createFrom(path, symbolToken);
}

void Monomorphizer::createAliasFrom(const Token& aliasToken, const Token& symbolToken)
{
    const std::string& genericModuleName = aliases.at(aliasToken.toString());

    createFrom(genericModuleName, symbolToken);
}

std::string Monomorphizer::getGenericModuleNameFromAlias(const Token& aliasToken)
{
    std::string genericModuleName;

    if (getMapElement<std::string, std::string>(aliases, aliasToken.toString(), genericModuleName))
        return genericModuleName;

    console.compileErrorOnToken("Use does not exist or NOT GENERIC", aliasToken);
}

std::string Monomorphizer::getGenericModuleNameFromStruct(const Token& nameToken)
{
    std::string genericModuleName;

    if (getMapElement<std::string, std::string>(genericUses, nameToken.toString(), genericModuleName))
        return genericModuleName;

    console.compileErrorOnToken("GENERIC STRUCT does not exist", nameToken);
}

Node Monomorphizer::instantiateGeneric(const std::string& genericModuleName,
                                       const std::string& instantiationSuffix,
                                       const Token& genericStructNameToken,
                                       const std::vector<icode::TypeDescription>& instantiationTypes,
                                       const std::vector<Node>& instantiationTypeNodes)
{
    const GenericASTIndex& index = genericsMap.at(genericModuleName);

    if (index.genericIdentifiers.size() != instantiationTypes.size())
        console.compileErrorOnToken("Number of type parameters don't match", genericStructNameToken);

    return instantiateAST(index,
                          genericStructNameToken,
                          instantiationTypes,
                          instantiationTypeNodes,
                          instantiationSuffix,
                          console);
}
