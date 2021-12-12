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

    GenericASTIndex index = GenericASTIndex{ast, genericIdentifiers, genericStructs};

    genericsMap[genericModuleName] = index;
}