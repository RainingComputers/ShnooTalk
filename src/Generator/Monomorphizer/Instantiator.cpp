#include "../../Builder/NameMangle.hpp"

#include "Instantiator.hpp"

Node constructNode(const node::NodeType nodeType, const std::string& tokenString)
{
    Node node;
    node.type = nodeType;
    node.tok = Token("", tokenString, token::GENERATED);

    return node;
}

void prependUseNode(Node& root, const std::string& moduleName, const std::string& alias)
{
    Node useNode = constructNode(node::USE, "use");
    useNode.children.push_back(constructNode(node::STR_LITERAL, '\"' + moduleName + '\"'));
    useNode.children.push_back(constructNode(node::IDENTIFIER, alias));

    root.children.insert(root.children.begin(), useNode);
}

Node stripModulesFromTypeNode(const Node& root) {}

Node prependModulesToTypeNode(const Node& root) {}

Node replaceGenericIdentifierInTypeNode(const Node& root, const Node& instantiationTypeNode) {}

bool validateTypeNode(const Node& root) {}

Node appendInstantiationSuffixToStruct(const Node& root, const std::string& instantiationSuffix) {}

void instantiateASTSingleGenericIdentifier(Node& genericModuleAST,
                                           std::string genericIdentifier,
                                           icode::TypeDescription instantiationType,
                                           Node typeDescriptionNode,
                                           const std::string& instantiationSuffix)
{
    const std::string& moduleName = instantiationType.moduleName;
    const std::string& alias = mangleModuleName(moduleName);

    prependUseNode(genericModuleAST, moduleName, alias);
}

void instantiateAST(Node& genericModuleAST,
                    const std::vector<std::string> genericIdentifiers,
                    std::vector<icode::TypeDescription> instantiationTypes,
                    std::vector<Node> typeDescriptionNodes,
                    const std::string& instantiationSuffix)
{


    for (int i = 0; i < genericIdentifiers.size(); i += 1)
    {
        instantiateASTSingleGenericIdentifier(genericModuleAST,
                                              genericIdentifiers[i],
                                              instantiationTypes[i],
                                              typeDescriptionNodes[i],
                                              instantiationSuffix);
    }
}