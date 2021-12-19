#include <algorithm>

#include "../../Builder/NameMangle.hpp"
#include "GenericASTIndex.hpp"
#include "MonomorphNameMangle.hpp"

#include "Instantiator.hpp"

struct InstiatorContext
{
    std::string& genericIdentifier;
    std::vector<std::string>& genericStructs;
    Token typeRootToken;
    Node instTypeNode;
    icode::TypeDescription instantiationType;
    std::string instantiationSuffix;
    Console& console;

    bool isGenericStruct(const Token& nameToken) const;
};

bool InstiatorContext::isGenericStruct(const Token& nameToken) const
{
    return std::find(genericStructs.begin(), genericStructs.end(), nameToken.toString()) != genericStructs.end();
}

Token modToken(const Token& tok, const std::string& tokenString)
{
    return Token(tok.getFileName(), tokenString, tok.getType());
}

Node constructNode(const node::NodeType nodeType, const std::string& tokenString)
{
    Node node;
    node.type = nodeType;
    node.tok = Token("", tokenString, token::GENERATED);

    return node;
}

void validateTypeNode(const InstiatorContext& ctx, Node& root)
{
    size_t childNodeCounter = 1;
    int typeModifierCount = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::IDENTIFIER, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::POINTER_STAR, childNodeCounter))
    {
        typeModifierCount++;
        childNodeCounter++;
    }

    if (root.isNthChild(node::EMPTY_SUBSCRIPT, childNodeCounter))
    {
        typeModifierCount++;
        childNodeCounter++;
    }

    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
        typeModifierCount++;

    if (typeModifierCount > 1)
        ctx.console.compileErrorOnToken("Invalid type parameters for this generic", ctx.typeRootToken);
}

void monomorphizeTypeNode(const InstiatorContext& ctx, Node& root)
{
    size_t childNodeCounter = 1;
    if (root.type == node::GENERIC_TYPE_PARAM)
        childNodeCounter = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter++;

    if (root.getNthChildToken(childNodeCounter).toString() == ctx.genericIdentifier)
    {
        root.children.erase(root.children.begin() + childNodeCounter);

        root.children.insert(root.children.begin() + childNodeCounter,
                             ctx.instTypeNode.children.begin(),
                             ctx.instTypeNode.children.end());

        validateTypeNode(ctx, root);

        return;
    }

    if (ctx.isGenericStruct(root.getNthChildToken(childNodeCounter)))
    {
        const Token& nameToken = root.children[childNodeCounter].tok;
        root.children[childNodeCounter].tok =
            modToken(nameToken, getInstantiatedStructName(ctx.instantiationSuffix, nameToken));

        return;
    }

    childNodeCounter++;

    while (root.isNthChild(node::GENERIC_TYPE_PARAM, childNodeCounter))
    {
        monomorphizeTypeNode(ctx, root.children[childNodeCounter]);
        childNodeCounter++;

        validateTypeNode(ctx, root.children[childNodeCounter]);
    }
}

void forLoop(const InstiatorContext& ctx, Node& root)
{
    if (root.isNthChild(node::VAR, 0))
        monomorphizeTypeNode(ctx, root.children[0]);
}

void statement(const InstiatorContext& ctx, Node& root)
{
    switch (root.type)
    {
        case node::VAR:
        case node::CONST:
            monomorphizeTypeNode(ctx, root);
            break;
        case node::FOR:
            forLoop(ctx, root);
            break;
        default:
            break;
    }
}

void block(const InstiatorContext& ctx, Node& root)
{
    for (Node& stmt : root.children)
        statement(ctx, stmt);
}

void function(const InstiatorContext& ctx, Node& root)
{
    monomorphizeTypeNode(ctx, root);

    for (size_t i = 1; root.children[i].isParamNode(); i += 1)
        monomorphizeTypeNode(ctx, root.children[i]);

    block(ctx, root.children.back());
}

void structure(const InstiatorContext& ctx, Node& root)
{
    for (Node& child : root.children[0].children)
        monomorphizeTypeNode(ctx, child);
}

void monomorphizeTypeNodes(const InstiatorContext& ctx, Node& root)
{
    switch (root.type)
    {
        case node::STRUCT:
            structure(ctx, root);
            break;
        case node::FUNCTION:
        case node::EXTERN_FUNCTION:
            function(ctx, root);
            break;
        case node::VAR:
            monomorphizeTypeNode(ctx, root);
            break;
        default:
            break;
    }
}

void appendInstantiationSuffixToStruct(const InstiatorContext& ctx, Node& root)
{
    const Token& nameToken = root.children[0].tok;

    if (!ctx.isGenericStruct(nameToken))
        return;

    root.children[0].tok = modToken(nameToken, getInstantiatedStructName(ctx.instantiationSuffix, nameToken));
}

void instantiateASTSingle(InstiatorContext& ctx, Node& genericModuleAST)
{
    for (Node& child : genericModuleAST.children)
    {
        monomorphizeTypeNodes(ctx, child);

        if (child.type == node::STRUCT)
            appendInstantiationSuffixToStruct(ctx, child);
    }
}

void prependUseNode(Node& root, const std::string& moduleName, const std::string& alias)
{
    Node useNode = constructNode(node::USE, "use");
    useNode.children.push_back(constructNode(node::STR_LITERAL, '\"' + moduleName + '\"'));
    useNode.children.push_back(constructNode(node::IDENTIFIER, alias));

    root.children.insert(root.children.begin(), useNode);
}

void stripModulesFromTypeNode(Node& root)
{
    int numModuleNodes = 0;

    while (root.isNthChild(node::MODULE, numModuleNodes))
        numModuleNodes++;

    root.children.erase(root.children.begin(), root.children.begin() + numModuleNodes);
}

void prependModuleToTypeNode(Node& root, const std::string& alias)
{
    root.children.insert(root.children.begin(), constructNode(node::MODULE, alias));
}

void prependUseNodes(const std::vector<icode::TypeDescription>& instantiationTypes,
                     std::vector<Node>& instTypeNodes,
                     Node& genericModuleAST)
{
    std::vector<std::string> prependedModules;

    for (size_t i = 0; i < instantiationTypes.size(); i += 1)
    {
        const icode::TypeDescription& type = instantiationTypes[i];
        Node& typeNode = instTypeNodes[i];

        const std::string& moduleName = type.moduleName;
        const std::string& alias = mangleModuleName(moduleName);

        stripModulesFromTypeNode(typeNode);
        prependModuleToTypeNode(typeNode, alias);

        if (std::find(prependedModules.begin(), prependedModules.end(), moduleName) != prependedModules.end())
            continue;

        prependUseNode(genericModuleAST, moduleName, alias);
    }
}

Node instantiateAST(GenericASTIndex index,
                    const Token& typeRootToken,
                    const std::vector<icode::TypeDescription>& instantiationTypes,
                    const std::vector<Node>& instTypeNodes,
                    const std::string& instantiationSuffix,
                    Console& console)
{
    Node& genericModuleAST = index.ast;

    for (size_t i = 0; i < index.genericIdentifiers.size(); i += 1)
    {
        InstiatorContext ctx =
            InstiatorContext{ index.genericIdentifiers[i], index.genericStructs, typeRootToken, instTypeNodes[i],
                              instantiationTypes[i],       instantiationSuffix,  console };

        instantiateASTSingle(ctx, genericModuleAST);
    }

    return genericModuleAST;
}
