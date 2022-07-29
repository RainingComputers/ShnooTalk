#include <algorithm>

#include "../../Utils/GetMapElement.hpp"
#include "../../Utils/ItemInList.hpp"
#include "../../Utils/KeyExistsInMap.hpp"
#include "Instantiator.hpp"

#include "Monomorphizer.hpp"

using namespace monomorphizer;

Monomorphizer::Monomorphizer(StringGenericASTMap& genericsMap, Console& console)
    : genericsMap(genericsMap)
    , console(console)
{
    resetWorkingModule();
}

void Monomorphizer::resetWorkingModule()
{
    workingModule = "";
}

std::vector<std::string> getGenericIdentifiers(const Node& root)
{
    std::vector<std::string> genericIdentifiers;

    for (const Node& child : root.children[0].children)
        genericIdentifiers.push_back(child.tok.toString());

    return genericIdentifiers;
}

bool genericTypePresent(const Node& root,
                        const std::vector<std::string>& genericIdentifiers,
                        const std::vector<std::string> genericStructs)
{
    if (root.type == node::IDENTIFIER)
    {
        if (itemInList<std::string>(root.tok.toString(), genericIdentifiers))
            return true;

        if (itemInList<std::string>(root.tok.toString(), genericStructs))
            return true;
    }

    for (const Node& child : root.children)
        if (genericTypePresent(child, genericIdentifiers, genericStructs))
            return true;

    return false;
}

std::pair<std::vector<std::string>, std::vector<std::string>> Monomorphizer::getGenericStructsAndFunctions(
    const Node& root,
    const std::vector<std::string>& genericIdentifiers)
{
    std::vector<std::string> genericStructs;
    std::vector<std::string> genericFunctions;

    for (const Node& child : root.children)
    {
        if (child.type != node::STRUCT && child.type != node::FUNCTION)
            continue;

        const Token nameToken = child.children[0].tok;

        if (itemInList<std::string>(nameToken.toString(), genericIdentifiers))
            console.compileErrorOnToken("Name cannot be a generic identifier", nameToken);

        if (genericTypePresent(child, genericIdentifiers, genericStructs))
        {
            if (child.type == node::STRUCT)
                genericStructs.push_back(nameToken.toString());

            if (child.type == node::FUNCTION)
                genericFunctions.push_back(nameToken.toString());
        }
    }

    return { genericStructs, genericFunctions };
}

void Monomorphizer::indexAST(const std::string& genericModuleName, const Node& ast)
{
    std::vector<std::string> genericIdentifiers = getGenericIdentifiers(ast);
    auto genericStructsAndFunctions = getGenericStructsAndFunctions(ast, genericIdentifiers);

    GenericASTIndex index =
        GenericASTIndex{ ast, genericIdentifiers, genericStructsAndFunctions.first, genericStructsAndFunctions.second };

    genericsMap[genericModuleName] = index;
}

void Monomorphizer::createUse(const Token& pathToken, const Token& aliasToken)
{
    const std::string path = pathToken.toUnescapedString();
    const std::string alias = aliasToken.toString();

    aliases[alias] = path;
    uses.push_back(path);
}

bool Monomorphizer::useExists(const Token& pathToken)
{
    const std::string path = pathToken.toUnescapedString();
    return itemInList<std::string>(path, uses);
}

bool Monomorphizer::aliasExists(const Token& aliasToken)
{
    return keyExistsInMap(aliases, aliasToken.toString());
}

void Monomorphizer::createFrom(const std::string& genericModuleName, const Token& symbolToken)
{
    GenericASTIndex index = genericsMap.at(genericModuleName);

    if (!index.isGenericStructOrFunction(symbolToken))
        console.compileErrorOnToken("Generic does not exist", symbolToken);

    genericUses[symbolToken.toString()] = genericModuleName;
}

void Monomorphizer::createDirectFrom(const Token& pathToken, const Token& symbolToken)
{
    const std::string path = pathToken.toUnescapedString();

    if (!useExists(path))
        uses.push_back(path);

    createFrom(path, symbolToken);
}

void Monomorphizer::createAliasFrom(const Token& aliasToken, const Token& symbolToken)
{
    const std::string genericModuleName = aliases.at(aliasToken.toString());

    createFrom(genericModuleName, symbolToken);
}

std::string Monomorphizer::getGenericModuleNameFromAlias(const Token& aliasToken)
{
    std::string genericModuleName;

    if (getMapElement<std::string, std::string>(aliases, aliasToken.toString(), genericModuleName))
        return genericModuleName;

    console.compileErrorOnToken("Use does not exist or not a generic module", aliasToken);
}

std::string Monomorphizer::getGenericModuleNameFromUse(const Token& nameToken)
{
    std::string genericModuleName;

    if (getMapElement<std::string, std::string>(genericUses, nameToken.toString(), genericModuleName))
        return genericModuleName;

    console.compileErrorOnToken("Generic does not exist", nameToken);
}

void Monomorphizer::setWorkingModuleFromAlias(const Token& aliasToken)
{
    workingModule = getGenericModuleNameFromAlias(aliasToken);
}

std::string Monomorphizer::getGenericModuleFromToken(const Token& token)
{
    if (workingModule.size() == 0)
        return getGenericModuleNameFromUse(token);

    const GenericASTIndex index = genericsMap.at(workingModule);

    if (!index.isGenericStructOrFunction(token))
        console.compileErrorOnToken("Generic does not exist", token);

    return workingModule;
}

bool Monomorphizer::genericExists(const Token& token)
{
    if (workingModule.size() == 0)
        return keyExistsInMap(genericUses, token.toString());

    const GenericASTIndex index = genericsMap.at(workingModule);

    return index.isGenericStructOrFunction(token);
}

Node Monomorphizer::instantiateGeneric(const std::string& genericModuleName,
                                       const Token& genericNameToken,
                                       const std::vector<icode::TypeDescription>& instantiationTypes,
                                       const std::vector<Node>& instantiationTypeNodes)
{
    const GenericASTIndex index = genericsMap.at(genericModuleName);

    if (index.genericIdentifiers.size() != instantiationTypes.size())
        console.compileErrorOnToken("Number of type parameters don't match", genericNameToken);

    return instantiateAST(index, genericNameToken, instantiationTypes, instantiationTypeNodes, console);
}
